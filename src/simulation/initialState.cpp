/**
 * @file initialState.cpp
 * @author Ryan Butler (rmbutler@outlook.com)
 * @brief Implements populateHighway for the derived classes of the InitialStateGenerator
 * @version 0.1
 * @date 2026-09-27
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <ranges>
#include <vector>
#include <fstream>
#include <filesystem>
#include <cerrno>
#include <cstring>
#include <charconv>
#include <database/DBReader.hpp>

#include "sim/initialState.hpp"

// File State utility functions

std::expected<std::unordered_map<size_t, FileState::Metadata>, std::string> FileState::getCarMetadata(std::filesystem::path csv, std::unordered_set<size_t>&& ids){
    std::unordered_map<size_t, FileState::Metadata> followData;

    std::string line;
    std::ifstream in(csv);
    if (!in.good()){
        // std::format_string fs{}
        return std::unexpected(std::format("Error opening {}: {}", csv.string(), std::strerror(errno))); 
    }

    std::getline(in, line);
    while (std::getline(in, line))
    {
        std::vector<double> data = line | std::views::split(',') 
        | std::views::transform([](auto part){
            std::string_view sv(part.begin(), part.end());
            double d{};
            std::from_chars(sv.data(), sv.data() + sv.size(), d);
            return d;
            }) 
        | std::ranges::to<std::vector>();
        if (data.size() != 6) 
            return std::unexpected(std::format("Expected 6 values per CSV row, got {}", data.size()));

        // [id, a, b, c, p, vdes]
        size_t id(data[0]);
        if (ids.contains(id)){
            FileState::Metadata follow{.a = data[1], .b = data[2], .c = data[3], .p = data[4], .vdes = data[4]};
            followData.insert({id, follow});
            ids.erase(id);
        }
    }
    if (ids.empty()){
        return followData;
    } else {
        return std::unexpected(std::format("Metadata for Car IDs {} not found in stats csv!",  ids));
    }
}

void FileState::populateHighway(const std::unordered_map<size_t, FileState::Metadata>& metadata, 
                                const std::unordered_map<size_t, FileState::SnapshotData>& snapshots,
                                std::shared_ptr<CarFactory> factory,
                                std::shared_ptr<Highway> hwy,
                                double t){
    
    // Sort the metadata and snapshot data by id. Could make the data structures maps initially and skip the expensive sort
    std::vector<size_t> ids = metadata | std::ranges::to<std::map>() | std::views::keys  | std::ranges::to<std::vector>();
    std::vector<FileState::Metadata> meta = metadata | std::ranges::to<std::map>() | std::views::values | std::ranges::to<std::vector>();
    std::vector<FileState::SnapshotData> snap = snapshots | std::ranges::to<std::map>() | std::views::values | std::ranges::to<std::vector>();

    // Need ID. Where do we get ID from? 
    for (auto [id, followData, snapshot] : std::views::zip(ids, meta, snap)){
        Car c = factory->makeCar(id, snapshot.x, snapshot.v, followData.vdes, t, followData.a, followData.b, followData.c, followData.p);
        hwy->addCar(std::move(c), snapshot.lane);
    }

}

IndividualFileState::IndividualFileState(std::filesystem::path directory):folder_{directory}{}

std::expected<void, std::string> IndividualFileState::populateHighway(std::shared_ptr<Highway> highway, double t, std::shared_ptr<CarFactory> factory){


    auto csvFiles = std::filesystem::directory_iterator(folder_) 
                | std::views::filter([](const auto& e) {
        return (e.path().extension() == ".csv") && (e.path().filename() != "car_stats.csv");
    }) | std::views::transform([](auto entry){return entry.path();})
       | std::ranges::to<std::vector>();
    // Store a map of the car IDs that are valid at t
    for (const std::filesystem::path& snapshotCsv : csvFiles){
        std::ifstream in(snapshotCsv);
        if (!in.good()){
            return std::unexpected(std::format("Error opening {}: {}", snapshotCsv.string(), std::strerror(errno)));
        }
        std::string line;
        std::getline(in, line); // eat the first line
        while(std::getline(in, line)){
            auto data = line | std::views::split(',') 
            | std::views::transform([](auto part){
                std::string_view sv(part.begin(), part.end());
                double d{};
                std::from_chars(sv.data(), sv.data() + sv.size(), d);
                return d;
                }) 
            | std::ranges::to<std::vector>();
            // data = [x, v, t, l]
            double time = data[2];
            int lane = int(data[3]);
            if (time == t){
                size_t id;
                std::string fname = snapshotCsv.filename().string();
                std::string_view sv(fname.begin(), fname.end());
                std::from_chars(sv.data(), sv.data()+sv.size(), id);
                FileState::SnapshotData snap {.x = data[0], .v = data[1], .lane =lane};
                m_cars.insert({id, snap});
                break;
            } else if (time > t){
                break;
            }
        }
    }

    // Have a map of the X, V, T positions for all valid IDs. Now need to parse the Car Metadata fields. 
    std::unordered_set<size_t> keys = m_cars | std::views::keys | std::ranges::to<std::unordered_set>();

    return FileState::getCarMetadata(folder_ / "car_stats.csv", std::move(keys)).transform([this, factory, highway, t](auto metadata){
        FileState::populateHighway(metadata, m_cars, factory, highway, t);
    });
}

TimeSeriesFileState::TimeSeriesFileState(std::filesystem::path directory):folder_{directory}{}

std::expected<void, std::string> TimeSeriesFileState::populateHighway(std::shared_ptr<Highway> highway, double t, std::shared_ptr<CarFactory> factory){
    // Read in a file called time400.0.csv
    // auto csv = 
    //             | std::views::filter([](const auto& e) {
    //     std::string filename = e.path().filename().string().substr(0, 5);
    //     std::string part = e.path().filename().string().substr(5); // always starts 
    //     return (e.path().extension() == ".csv") && (e.path().filename() != "car_stats.csv");
    // }) | std::views::transform([](auto entry){return entry.path();})
    //    | std::ranges::to<std::vector>();

    auto csv = std::ranges::find_if(std::filesystem::directory_iterator(folder_),
    [t](const auto& e){
        std::string file = e.path().filename().string();
        if (file.substr(0, 5) != "time_"){
            return false;
        } else {
            std::string str = file.substr(5);
            std::string_view sv(str.begin(), str.end());
            double time{};
            std::from_chars(sv.data(), sv.data() + sv.size(), time);
            return time == t;
        }
    });
    if (csv == std::filesystem::directory_iterator{}){
        return std::unexpected(std::format("Unable to find a csv with the timestamp {}", t));
    }

    std::ifstream in(csv->path());
    if (!in.good()){
        return std::unexpected(std::format("Error opening {}: {}", csv->path().string(), std::strerror(errno)));
    }

    std::unordered_map<size_t, FileState::SnapshotData> cars;
    std::string line;
    std::getline(in, line); // eat the first line
    while(std::getline(in, line)){
        auto data = line | std::views::split(',') 
        | std::views::transform([](auto part){
            std::string_view sv(part.begin(), part.end());
            double d{};
            std::from_chars(sv.data(), sv.data() + sv.size(), d);
            return d;
            }) 
        | std::ranges::to<std::vector>();
        // data = [id, x, v, l]
        double time = data[2];
        int lane = int(data[3]);
        if (time == t){
            size_t id(data[0]);
            FileState::SnapshotData snap {.x = data[0], .v = data[1], .lane =lane};
            cars.insert({id, snap});
            break;
        } else if (time > t){
            break;
        }
    }

    std::unordered_set<size_t> keys = cars | std::views::keys | std::ranges::to<std::unordered_set>();
    return FileState::getCarMetadata(folder_ / "car_stats.csv", std::move(keys)).transform([cars, factory, highway, t](auto metadata){
        FileState::populateHighway(metadata, cars, factory, highway, t);
    });
}

DatabaseFileState::DatabaseFileState(std::string jobname, bool useTestDB):
    jobname_{jobname}, useTestDB_{useTestDB}{}

std::expected<void, std::string> DatabaseFileState::populateHighway(std::shared_ptr<Highway> highway, double t, std::shared_ptr<CarFactory> factory) {

    DBReader reader(useTestDB_);
}
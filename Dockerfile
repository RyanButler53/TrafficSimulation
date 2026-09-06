FROM alpine:3.23

# want to bring in yaml, python is probably there

RUN apk update

# install C++, Cmake and lldb debugger
RUN apk add clang && apk add cmake>3.31.0
RUN apk add lldb 
RUN apk add git

# Get the yaml parsing
RUN apk add yaml-cpp

# Get the python stuff
RUN apk add python3 
RUN apk add ipython


RUN apk add git

# Get git

WORKDIR /traffic

COPY . /traffic

CMD ["git", "--version"]


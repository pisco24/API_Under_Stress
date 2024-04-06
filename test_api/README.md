# Test API 
## Dependencies
* Crow source code
* C++ compiler with at least C++11 support (xCode Command Line Tools on Mac)
* Asio development headers (1.10.9 or later):
* Boost

Get Crow: 
```> git clone https://github.com/CrowCpp/Crow.git```

Get Asio:```> brew install cmake asio```


Get Boost: ```> brew install boost```


## Installation

### Fix Crow header $Paths
In the "include" folder there is another "crow" folder with all of the header files, they link to each other with #include but each pathway from one .h file to another looks like this: "crow/name_of_header.h". You need to remove the "crow/" from every link in every .h file. I did this the slow and annoying way, but maybe you can find a fast solution (?)

### Compile API
From the test_api folder, compile main.cpp:

```> g++ -std=c++11 main.cpp -o main ```

## Usage
Run the api from the test_api folder:

```> ./main ```

Check the local host to see that it is working at:
http://localhost:18080



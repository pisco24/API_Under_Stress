# API Under Stress

## Objective

To design, develop, and test a resilient API that can effectively handle high loads,
unexpected spikes in traffic, and various types of stress scenarios, ensuring minimal
service disruption and consistent performance.

## Constraints

- CPU Constraints: 1.5 CPUs
- Memory Constraints: 3.0 GB

## Stack/Technologies

- Crow web framework (https://crowcpp.org/master/) for C++ 
- MongoDB
- Nginx (load balancer) 
- Docker
- Gatling (https://gatling.io/) used to perform stress tests on the API.

## Usage

Git clone to download the repo to your local environment:
```
git clone https://github.com/pisco24/API_Under_Stress.git
```

Assuming you have Docker Engine installed and running, run these docker commands in the project directory:
```
docker compose build
docker compose up -d
```
The API should be running (in Docker containers) now.


### There are four endpoints for this API:

- POST /warrior – creates warrior, request body includes data for the warrior in a JSON format, all listed fields are mandatory:
    - name, string max 100 characters.
    - dob (Date of Birth), string to date in format AAAA-DD-MM (year, day, month).
    - fight_skills, list/array with max 20 entries of strings max 250 characters
    Response header returns a unique url w/ id that can be used with the GET /warrior/[:id] endpoint. 

- GET /warrior/[:id] – returns warrior created with corresponding id

- GET /warrior?t=[:search term] – searches warrior attributes

- GET /counting-warriors – returns number of warriors that are in the database;


### Examples (using curl):

POST /warrior endpoint:
```
curl -v http://localhost:8080/warrior -H 'Content-Type: application/json' -d '{"name":"Muhammed Ali","dob":"1942-01-17", "fight_skills":["Boxing"]}'
```

GET /warrior/[:id] endpoint:
```
curl -v http://localhost:8080/warrior/6639ae4150fedcddb10501bd
```
GET /warrior?t=[:search_term]:
```
curl -v http://localhost:8080/warrior?t=Boxing
```

GET /counting-warriors:
```
curl -v http://localhost:8080/counting-warriors
```

## Results

#### Stress test results produced by Gatling:
[2024-05-07 17:32:18 GMT](https://htmlpreview.github.io/?https://github.com/pisco24/API_Under_Stress/blob/main/sample_result/index.html)

Note: Actual results may vary depending on operating system, hardware, etc.

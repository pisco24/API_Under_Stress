#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>

// Your existing code
std::string name = json_body["name"].s();  
std::string dob = json_body["dob"].s(); 
auto fight_skills = const_cast<crow::json::rvalue&> (json_body["fight_skills"]).lo();

// Create a document builder
bsoncxx::builder::stream::document document{};

// Add fields to the document
document << "name" << name
         << "dob" << dob
         << "fight_skills" << bsoncxx::builder::stream::open_array;

// Add fight skills to the document
for (const auto& skill : fight_skills) {
    document << skill.s();
}

document << bsoncxx::builder::stream::close_array;

// Get a collection handle
auto collection = db[collection_name];

// Insert the document into the collection
bsoncxx::stdx::optional<mongocxx::result::insert_one> result = collection.insert_one(document.view());

// Check if the insert was successful
if(result) {
    std::cout << "Document inserted successfully." << std::endl;
} else {
    std::cout << "Document insertion failed." << std::endl;
}

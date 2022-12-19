#include <iostream>
#include <fstream>
#include <string>
#include "json.hpp"

using json = nlohmann::json;


auto getRelevantSessions(){
    
}


int main(){
    // Creates                                                                                                                                                                                                                               JSON object from json file. 
    // Information in file obtained from Postman HTTP GET request
    std::ifstream f("testJSON.json");
    json flowData = json::parse(f);

    json resultsData;

    for (auto person : flowData["participantInfo"]) {
        resultsData["id"] = person["participantId"];
        resultsData["name"] = person["name"];

        
        auto partSessions = person["sessions"];

        for (auto s : partSessions){
            for (auto session : flowData["sessions"]){
                if (session["sessionId"] == s){

                }
            }
        }

    }




    return 0;
}
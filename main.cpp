#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include "json.hpp"

using json = nlohmann::json;


int main(){
    // Creates JSON object from json file. 
    // Information in file obtained from Postman HTTP GET request
    std::ifstream f("testJSON.json");
    json flowData = json::parse(f);

    json resultsData = json::array();

    // runs on every person in participant list 
    for (auto person : flowData["participantInfo"]) {
        // creates a new json object representing a participant 
        json newParticipant;

        // gives the newly created object the name and id of the current person
        newParticipant["id"] = person["participantId"];
        newParticipant["name"] = person["name"];

        //creates an empty json array for the participant's lanuages 
        json languages = json::array();

        int totalSessions = person["sessions"].size();


        int checkedSessions = 0;
        // double totalSessionDuration = 0;
        // int numberOfSessions = 0;
        
        for (auto s : flowData["sessions"]){
            if (checkedSessions == totalSessions){
                break;
            }

            if (s["participantId"] == newParticipant["id"]){
                json newLang;
                newLang["language"] = s["language"];

                languages += newLang;
            }

            newParticipant["languages"] = languages;
        }

        resultsData += newParticipant;

    }

    std::cout << resultsData.dump(4) << std::endl;
    

    return 0;
}
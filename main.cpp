#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

bool alreadyAdded(std::vector<std::string> seen, std::string lang){
    for (std::string each : seen){
        if (lang.compare(each) == 0){
            return true;
        }
    }
    return false;
}

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


        // finds the number of sessions a person has done
        int totalSessions = person["sessions"].size();
        int checkedSessions {};
        double totalSessionDuration {};
        std::vector<std::string> seenLanguages {};


        //double overallAverageRoundScore {};


        
        for (auto s : flowData["sessions"]){
            double scorePerLang {}; 
            double roundDurPerLang {};
            

            //if all of a participants sessions have been checked, end the loop without going through the rest
            if (checkedSessions == totalSessions){
                break;
            }
 
            if (s["participantId"] == newParticipant["id"]){
                json newLang;
                
                if (alreadyAdded(seenLanguages, s["language"])){
                    
                    
                } else {
                    newLang["language"] = s["language"];
                    seenLanguages.push_back(s["language"]);
                    
                    int checkedRounds {};
                    int totalRounds = s["rounds"].size();

                    // determines the relevant round information for the current language 
                    for (auto r : s["rounds"]){
                        for (auto round : flowData["rounds"]){
                            if (checkedRounds == totalRounds){
                                break;
                            }
                            if (r == round["roundId"]){
                                checkedRounds++;
                                int score = round["score"];
                                scorePerLang += score;
                                
                                
                                double begining = round["startTime"];
                                double ending = round["endTime"];
                                roundDurPerLang += (ending - begining);
                            }
                        }
                    }
                    newLang["averageScore"] += (scorePerLang / totalRounds);
                    newLang["averageRoundDuration"] += (roundDurPerLang / totalRounds);

                    
                    

                    languages += newLang;
                }
            
                double begining = s["startTime"];
                double ending = s["endTime"];

                totalSessionDuration += (ending - begining);
                checkedSessions++;
            }

            
        }


        if (checkedSessions > 0){
                newParticipant["languages"] = languages;
                double aveDuration = totalSessionDuration / checkedSessions;
                newParticipant["averageSessionDuration"] = ceil(aveDuration * 100.0) / 100.0;
            } else {
                newParticipant["averageSessionDuration"] = "N/A";
            }

        resultsData += newParticipant;

    }

    std::cout << resultsData.dump(4) << std::endl;
    

    return 0;
}
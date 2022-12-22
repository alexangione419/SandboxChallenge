#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "json.hpp"

using json = nlohmann::ordered_json;

bool alreadyAdded(std::vector<std::string> seen, std::string lang){
    for (std::string each : seen){
        if (lang.compare(each) == 0){
            return true;
        }
    }
    return false;
}

double sum(std::vector<double> list){
    double result {};
    for (double each : list){
        result += each;
    }
    return result;
}

int main(){
    // Creates JSON object from json file. 
    // Information in file obtained from Postman HTTP GET request
    std::ifstream f("sandboxJSON.json");
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


        double overallAverageRoundScore {};
        double overallTotalRounds {};


        
        for (auto s : flowData["sessions"]){
            std::vector<double> scorePerLang {}; 
            std::vector<double> roundDurPerLang {};
            json newLang = json::object();
            

            //if all of a participants sessions have been checked, end the loop without going through the rest
            if (checkedSessions == totalSessions){
                break;
            }
 
            if (s["participantId"] == newParticipant["id"]){
                int checkedRounds {};

                // determines the relevant round information for the current language 
                for (auto r : s["rounds"]){
                    for (auto round : flowData["rounds"]){
                        if (r == round["roundId"]){
                            checkedRounds++;
                            double score = round["score"];
                            scorePerLang.push_back(score);

                            overallAverageRoundScore += score;
                            overallTotalRounds++;
                            
                            
                            double begining = round["startTime"];
                            double ending = round["endTime"];
                            roundDurPerLang.push_back(ending - begining);
                        }
                        
                    }
                }

                if (alreadyAdded(seenLanguages, s["language"])){
                    int numOfLangs = languages.size();                    
                    for (int i = 0; i < numOfLangs; i++){
                        std::string currLang {languages[i]["language"]};

                        if (currLang.compare(s["language"]) == 0){
                            std::vector<double> roundScores = languages[i]["averageScore"];
                            roundScores.insert(roundScores.end(), scorePerLang.begin(), scorePerLang.end());
                            languages[i]["averageScore"] = roundScores;

                            std::vector<double> roundLengths = languages[i]["averageRoundDuration"];
                            roundLengths.insert(roundLengths.end(), roundDurPerLang.begin(), roundDurPerLang.end());
                            languages[i]["averageRoundDuration"] = roundLengths;
                        }
                    }
                    
                } else {
                    newLang["language"] = s["language"];
                    seenLanguages.push_back(s["language"]);
                    
                    newLang["averageScore"] = scorePerLang;
                    newLang["averageRoundDuration"] = roundDurPerLang;

                    languages += newLang;
                }
            
                double begining = s["startTime"];
                double ending = s["endTime"];

                totalSessionDuration += (ending - begining);
                checkedSessions++;
            }
   
        }
        newParticipant["languages"] = languages;


        //Once every round's score has been listen under each language, the are averaged together
        int partLangs = newParticipant["languages"].size();
        for (int i = 0; i < partLangs; i++){
            try {
                std::vector<double> scores = newParticipant["languages"][i]["averageScore"];
                double aveScore = sum(scores) / scores.size();
                newParticipant["languages"][i]["averageScore"] = ceil(aveScore * 100.0) / 100.0;

                std::vector<double> lengths = newParticipant["languages"][i]["averageRoundDuration"];
                double aveLength = sum(lengths) / lengths.size();
                newParticipant["languages"][i]["averageRoundDuration"] = ceil(aveLength * 100.0) / 100.0;
                
                throw "Not a vector";
            } catch(...) {

            }
            
        }

        // Calcualtes the average round score, if there is something to calculate
        if (overallTotalRounds > 0){
            double aveScore = overallAverageRoundScore / overallTotalRounds;
            newParticipant["averageRoundScore"] = ceil(aveScore * 100.0) / 100.0;
        } else {
            newParticipant["averageRoundScore"] = "N/A";
        }
        

        if (checkedSessions > 0){
                double aveDuration = totalSessionDuration / checkedSessions;
                newParticipant["averageSessionDuration"] = ceil(aveDuration * 100.0) / 100.0;
            } else {
                newParticipant["averageSessionDuration"] = "N/A";
            }

        resultsData += newParticipant;

    }

    std::ofstream results("participantResults.json");
    results << resultsData.dump(2);
    results.close();

    


    return 0;
}
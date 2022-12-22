#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include "json.hpp"

//ordered_json ensure that json is created with keys in order of addition
using json = nlohmann::ordered_json;

//determines if a string value is already within a vector
bool alreadyAdded(std::vector<std::string> seen, std::string lang){
    for (std::string each : seen){
        if (lang.compare(each) == 0){
            return true;
        }
    }
    return false;
}

//computes the sum of a vector of doubles
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

    //json to hold final data
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

        //variables to hold information collected for each person 
        int checkedSessions {};
        double totalSessionDuration {};
        std::vector<std::string> seenLanguages {};
        double overallAverageRoundScore {};
        double overallTotalRounds {};


        //checks every session
        for (auto s : flowData["sessions"]){
            // the score and duration values for each langauge are held in a vector
            // this is so that if a language that we already saw comes up later, we can just add its 
            // information to the vector
            // If we tried to calculate the average round score and duration every time we saw a language, we would get
            // incorrect values in the end
            std::vector<double> scorePerLang {}; 
            std::vector<double> roundDurPerLang {};
            json newLang = json::object();
            

            //if all of a participants sessions have been checked, end the loop without going through the rest
            if (checkedSessions == totalSessions){
                break;
            }

            //code evaluates if the participant was involved in the current session
            if (s["participantId"] == newParticipant["id"]){
                int checkedRounds {};

                // determines the relevant round information for the current language 
                for (auto r : s["rounds"]){
                    for (auto round : flowData["rounds"]){
                        //goes through every round, and when it finds one the participant was apart of, 
                        // it retains the score and duration
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
                // runs if we come across a langauge that we already have data for
                if (alreadyAdded(seenLanguages, s["language"])){
                    int numOfLangs = languages.size();                    
                    for (int i = 0; i < numOfLangs; i++){
                        std::string currLang {languages[i]["language"]};
                        //adds the new language information to the previous data collected
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
                    // creates a new lanuage in the list of languages
                    newLang["language"] = s["language"];
                    seenLanguages.push_back(s["language"]);
                    
                    newLang["averageScore"] = scorePerLang;
                    newLang["averageRoundDuration"] = roundDurPerLang;

                    //code below attempts to order the languages properly
                    //I am unsure what order the languages are desired in, and since everything else evaluates properly I will
                    //  leave it how it is
                    json tempLanguage = json::array();
                    tempLanguage += newLang;

                    tempLanguage.insert(tempLanguage.end(), languages.begin(), languages.end());
                    std::reverse(tempLanguage.begin(), tempLanguage.end());

                    languages = tempLanguage;
                   

                }
                //computes the total duration of the current session, and registers we have 
                // checked another section
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
            //this try catch ensures that because some values are integers and some are vectors, 
            // this code will only run on vectors and correctly do nothing to integers

            //this is where, since we have seen every language, the data vectors can be added together, 
            // as there will be no new information to consider
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
        
        // calculates the average session duration, if there is data to calculate
        if (checkedSessions > 0){
                double aveDuration = totalSessionDuration / checkedSessions;
                newParticipant["averageSessionDuration"] = ceil(aveDuration * 100.0) / 100.0;
            } else {
                newParticipant["averageSessionDuration"] = "N/A";
            }

        resultsData += newParticipant;

    }

    //outputs the created json to a clean json file
    std::ofstream results("participantResults.json");
    results << resultsData.dump(2);
    results.close();


    return 0;
}
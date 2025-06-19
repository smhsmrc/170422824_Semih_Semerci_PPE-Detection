#pragma once
#ifndef TRACKERFACTORY_H
#define TRACKERFACTORY_H

#include "ITracker.h"
#include "SKalmanTracker.h"
#include "OCSort.h"
#include "centroidtracker.h"
#include "BYTETracker.h"
// #include "DStracker.h"
#include <memory>
#include <stdexcept>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>

class TrackerConfig {
public:
    std::map<std::string, std::string> configMap;

    void loadConfig(const std::string& filePath) {
        std::ifstream file(filePath);
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream is_line(line);
            std::string key;
            if (std::getline(is_line, key, '=')) {
                std::string value;
                if (std::getline(is_line, value)) {
                    configMap[key] = value;
                }
            }
        }
    }

    std::string get(const std::string& key, const std::string& defaultValue = "") const {
        auto it = configMap.find(key);
        if (it != configMap.end()) {
            return it->second;
        }
        return defaultValue;
    }
};

TrackerConfig get_tracker_config(const std::string& method) {
    TrackerConfig cfg;
    std::string configPath =  "./configs/" + method + ".txt"; // Assuming .txt file instead of .yaml
    cfg.loadConfig(configPath);
    return cfg;
}

enum class TrackerType {
    Sort,
    OCSort,
    Centroid,
    BYTETracker,
    DeepSort
    // Add other tracker types
};

class TrackerFactory {
public:
    static std::unique_ptr<ITracker> createTracker(TrackerType type) {
        std::string methodName = trackerTypeToString(type);
        TrackerConfig cfg = get_tracker_config(methodName);
        switch (type) {
        case TrackerType::Sort:
            return std::unique_ptr<ITracker>(new KalmanTracker());
        case TrackerType::OCSort:
            return std::unique_ptr<ITracker>(new ocsort::OCSort(
                std::stoi(cfg.get("detThresh", "0")),
                std::stoi(cfg.get("maxAge", "50")),
                std::stod(cfg.get("minHits", "1")),
                std::stod(cfg.get("iouThreshold", "0.22136877277096445")),
                std::stoi(cfg.get("deltaT", "1")),
                cfg.get("assoFunc", "giou"),
                std::stod(cfg.get("inertia", "0.3941737016672115")),
                cfg.get("useByte", "true") == "true"));
        case TrackerType::Centroid:
            return std::unique_ptr<ITracker>(new CentroidTracker(std::stoi(cfg.get("maxDisappeared", "20"))));
        case TrackerType::BYTETracker:
            return std::unique_ptr<ITracker>(new BYTETracker(std::stoi(cfg.get("frameRate", "30")), std::stoi(cfg.get("trackingBuffer", "20"))));
        // case TrackerType::DeepSort:
        //     return std::unique_ptr<ITracker>(new tracker(std::stoi(cfg.get("maxCosineDistance", "0.2")), std::stoi(cfg.get("nnBudget", "100"))));
        default:
            throw std::invalid_argument("Unknown tracker type");
        }
    }
    static std::string trackerTypeToString(TrackerType type) {
        switch (type) {
        case TrackerType::Sort: return "Sort";
        case TrackerType::OCSort: return "OCSort";
        case TrackerType::Centroid: return "Centroid";
        case TrackerType::BYTETracker: return "BYTETracker";
        // case TrackerType::DeepSort: return "DeepSort";
        default: throw std::invalid_argument("Unknown tracker type");
        }
    }
};

#endif // TRACKERFACTORY_H

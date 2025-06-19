#include "../include/progressbar.h"
#include <iostream>
#include <opencv2/opencv.hpp>

// Constructor to initialize the progress bar
ProgressBar::ProgressBar(int maxVal, cv::Rect r)
    : maxValue(maxVal), currentValue(0), state(ProgressBarState::IN_PROGRESS),
      rect(r) {}

// Function to update the progress bar
void ProgressBar::updateProgress(int value, bool fullyProtected) {
    if (fullyProtected) {
        state = ProgressBarState::COMPLETED;  // If fully protected, set to completed
        color = cv::Scalar(0, 255, 0);  // Green color for completed state
        return;  // No need to update further
    }
    else {
        state = ProgressBarState::IN_PROGRESS;  // If not fully protected, set to in progress
        color = cv::Scalar(0, 0, 255);  // Blue color for in progress state
    }
    if (value >= maxValue) {
        state = ProgressBarState::COMPLETED;
        color = cv::Scalar(0, 0, 255);  // Green color for completed state
        return;  // Ignore if value exceeds maxValue
    }
    if (value < 0) {
        state = ProgressBarState::ERROR;  // Error if value is out of bounds
        std::cout << "Error: Value out of range!" << std::endl;
    } else {
        currentValue = value;
    }
}

// Function to draw the progress bar on a cv::Mat
void ProgressBar::drawProgress(cv::Mat &image, bool fullyProtected) const {
    // Optionally, add text
    std::string progressText = std::to_string(currentValue) + "/" + std::to_string(maxValue);
    
    if (state == ProgressBarState::COMPLETED) {
        cv::rectangle(image, rect, color, -1);  // Green for completed
        cv::putText(image, progressText, cv::Point(rect.x + 5, rect.y + rect.height / 2), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
        return;  // No need to draw the filled part if completed
    } 
    
    // Draw the background rectangle (gray)
    cv::rectangle(image, rect, cv::Scalar(200, 200, 200), -1);  // Filled background
    
    // Calculate the width of the filled progress part
    int filledWidth = static_cast<int>((static_cast<double>(currentValue) / maxValue) * rect.width);
    
    // Draw the filled rectangle (colored)
    cv::rectangle(image, cv::Rect(rect.x, rect.y, filledWidth, rect.height), color, -1);  // Filled progress
    
    cv::putText(image, progressText, cv::Point(rect.x + 5, rect.y + rect.height / 2), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 1);
}

// Function to display the current progress (to console)
void ProgressBar::displayProgress() const {
    std::cout << "Progress: " << currentValue << "/" << maxValue << " (" 
              << getStateString() << ")\n";
}

// Helper function to convert state to string for display
std::string ProgressBar::getStateString() const {
    switch (state) {
        case ProgressBarState::IN_PROGRESS:
            return "In Progress";
        case ProgressBarState::COMPLETED:
            return "Completed";
        case ProgressBarState::ERROR:
            return "Error";
        default:
            return "Unknown State";
    }
}

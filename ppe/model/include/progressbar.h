#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <opencv2/opencv.hpp>
#include <string>

// Enum to represent the state of the progress bar
enum class ProgressBarState {
    IN_PROGRESS,  // When the progress bar is in progress
    COMPLETED,    // When the progress is finished
    ERROR         // When there is an error in progress
};

// Struct to represent a progress bar
struct ProgressBar {
    int maxValue;         // Maximum value of the progress bar
    int currentValue;     // Current value of the progress bar
    ProgressBarState state;  // Current state of the progress bar
    cv::Rect rect;        // The bounding box of the progress bar on the image
    cv::Scalar color;     // Color of the progress bar

    // Constructor to initialize the progress bar
    ProgressBar(int maxVal, cv::Rect r);

    // Function to update the progress bar
    void updateProgress(int value, bool fullyProtected);

    // Function to draw the progress bar on a cv::Mat image
    void drawProgress(cv::Mat &image, bool fullyProtected) const;

    // Function to display the current progress (to console)
    void displayProgress() const;

private:
    // Helper function to convert state to string for display
    std::string getStateString() const;
};

#endif // PROGRESSBAR_H

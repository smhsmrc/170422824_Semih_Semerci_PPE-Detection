#ifndef PERSON_H
#define PERSON_H

#include <string>
#include <opencv2/opencv.hpp>

struct Person {
    cv::Mat frame;
    cv::Rect detected_roi;
    int detectedFrameId;   // The ID of the frame where the person was detected
    int trackerId;         // The tracker's ID for identifying the person
    bool helmet;           // True if the person is wearing a helmet
    bool vest;             // True if the person is wearing a vest
    bool gloves;       // True if the person is wearing a right glove
    // bool rightGlove;       // True if the person is wearing a right glove
    // bool leftGlove;        // True if the person is wearing a left glove
    bool mask;             // True if the person is wearing a mask
    int cardId;
    bool card_status;
    bool notified = false;

    // Constructor to initialize a Person object with default values
    Person(int frameId = 0, int tId = 0, bool h = false, bool v = false, 
           bool g = false, bool m = false);

    // Function to update the attributes of a Person
    void updatePerson(int frameId, int tId, bool h, bool v, bool g, bool m);

    // Function to check if the person is wearing all required PPE
    bool isFullyProtected() const;

    // Function to display the state of the person (for debugging/logging purposes)
    void displayPerson() const;
};

#endif // PERSON_H

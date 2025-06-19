#include "../include/person.h"
#include "../../ui/pages/include/settings.h"
#include <iostream>

// Constructor to initialize a Person object with default values
Person::Person(int frameId, int tId, bool h, bool v, bool g, bool m)
    : detectedFrameId(frameId), trackerId(tId), helmet(h), vest(v),
      gloves(g), mask(m) {}

// Function to update the attributes of a Person
void Person::updatePerson(int frameId, int tId, bool h, bool v, bool g, bool m) {
    detectedFrameId = frameId;
    trackerId = tId;
    helmet = h;
    vest = v;
    gloves = g;
    mask = m;
}

// Function to check if the person is wearing all required PPE
bool Person::isFullyProtected() const {
    // KKD ayarlarını al
    std::map<std::string, bool> kKDSettings = Settings::getKKDSettings();
    
    // Sadece aktif olan KKD'leri kontrol et
    bool helmetRequired = kKDSettings["helmet"];
    bool vestRequired = kKDSettings["vest"];
    bool glovesRequired = kKDSettings["gloves"];
    bool maskRequired = kKDSettings["mask"];
    
    // Gerekli KKD'lerin hepsinin takılı olup olmadığını kontrol et
    bool helmetOK = !helmetRequired || helmet;
    bool vestOK = !vestRequired || vest;
    bool glovesOK = !glovesRequired || gloves;
    bool maskOK = !maskRequired || mask;
    
    return helmetOK && vestOK && glovesOK && maskOK;
}

// Function to display the state of the person (for debugging/logging purposes)
void Person::displayPerson() const {
    std::cout << "Person Details:" << std::endl;
    std::cout << "Detected Frame ID: " << detectedFrameId << std::endl;
    std::cout << "Tracker ID: " << trackerId << std::endl;
    std::cout << "Helmet: " << (helmet ? "Yes" : "No") << std::endl;
    std::cout << "Vest: " << (vest ? "Yes" : "No") << std::endl;
    std::cout << "Gloves: " << (gloves ? "Yes" : "No") << std::endl;
    // std::cout << "Right Glove: " << (rightGlove ? "Yes" : "No") << std::endl;
    // std::cout << "Left Glove: " << (leftGlove ? "Yes" : "No") << std::endl;
    std::cout << "Mask: " << (mask ? "Yes" : "No") << std::endl;
}

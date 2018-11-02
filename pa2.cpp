#include "pa2.h"
#include <iostream>
#include <limits>
#include <cmath>

// organize chunk of pages that belong to a program
// start and end bounds are exclusive
Chunk::Chunk(std::string name, int startBound, int endBound) {
    startPage = startBound;
    endPage = endBound;
    programName = std::move(name);
}

int Chunk::getStartPage() {
    return startPage;
}

int Chunk::getEndPage() {
    return endPage;
}

void Chunk::setStartPage(int page) {
    startPage = page;
}

void Chunk::setEndPage(int page) {
    endPage = page;
}

std::string Chunk::getProgramName() {
    return programName;
}

// start and end pages are exclusive
int Chunk::getSize() {
    return endPage - startPage - 1;
}

// shift the starting page in a chunk in memory by a certain number
void Chunk::split(int size) {
    startPage += size;
}

// print out the programs in 8 column rows
void Chunk::print() {
    for(int i = 1; i < endPage; ++i) {
        std::cout << programName;
        if(i % 8 == 0) {
            std::cout << std::endl;
        } else if (i != endPage - 1){
            std::cout << '\t';
        }
    }
}

// represent pages with strings that are their names
OperatingSystem::OperatingSystem(std::string algorithmChosen) {
    this->algorithmChosen = std::move(algorithmChosen);
    usedSpace = LinkedList();
    freeSpace = LinkedList();
    // create one initial node in the free memory containing all the free space
    // leave used memory empty
    freeSpace.append(Chunk("Free", 0, 33));
}

// check if list of active programs contains program of interest
bool OperatingSystem::containsProgram(std::string programName) {
    for(const std::string& program : programsList) {
        if(program == programName) {
            return true;
        }
    }
    return false;
}

// best fit: find smallest hole that can fit the program size
// worst fit: find largest hole overall
// 4 KB per page
void OperatingSystem::addProgram(std::string programName, int programSize) {
    // make sure program isn't already running
    if(containsProgram(programName)) {
        std::cout << "Error, program " << programName << " already running.";
        return;
    }

    // number of total pages needed is the ceiling of the program size divided by 4
    auto numPages = (int) ceil((double) programSize / 4.0);

    int pageIndex = -1;
    // use worst or best case algorithm
    if(algorithmChosen == "worst") {
        // find free space chunk with most size
        int tempSize;
        // max size can't be anything smaller than 0
        int maxSize = 0;
        for(int i = 0; i < freeSpace.getSize(); ++i) {
            tempSize = 4 * freeSpace.get(i).getSize();
            if (tempSize > maxSize && tempSize >= programSize) {
                maxSize = tempSize;
                pageIndex = i;
            }
        }
        // no chunk has enough memory
        if(pageIndex < 0) {
            std::cout << "Error, not enough memory for Program " << programName;
            return;
        }
    } else {
        // find free space chunk with smallest size that fits the program size
        int tempSize;
        // minimum size can't be any larger than the OS size
        int minSize = 129;
        for(int i = 0; i < freeSpace.getSize(); ++i) {
            tempSize = 4 * freeSpace.get(i).getSize();
            if(tempSize >= programSize && tempSize < minSize && tempSize >= programSize) {
                minSize = tempSize;
                pageIndex = i;
            }
        }
        // no chunk has enough memory
        if(pageIndex < 0) {
            std::cout << "Error, not enough memory for Program " << programName;
            return;
        }
    }

    // get the target chunk and create a chunk in used memory
    // shift the starting index of the chunk if free memory by the number of pagegs
    Chunk targetChunk = freeSpace.get(pageIndex);
    Chunk* usedMemChunk = new Chunk(programName, targetChunk.getStartPage(), targetChunk.getStartPage() + numPages + 1);
    freeSpace.get(pageIndex).split(numPages);

    // insert chunk in used space in order by comparing start pages
    // if used memory is empty simply append it with a chunk as its head node
    if(usedSpace.getSize() == 0) {
        usedSpace.append(*usedMemChunk);
    } else if(usedSpace.getSize() == 1) {
        // if single chunk start page is greater than reclaimed start page, insert reclaimed chunk at front
        if(usedSpace.get(0).getStartPage() > usedMemChunk->getStartPage()) {
            usedSpace.insert(0, *usedMemChunk);
            // else add reclaimed chunk after the head node
        } else {
            usedSpace.append(*usedMemChunk);
        }
    }
        // else find best position in list
    else {
        int usedIndex;
        for (usedIndex = 1; usedIndex < usedSpace.getSize(); ++usedIndex) {
            if (usedSpace.get(usedIndex).getStartPage() < usedMemChunk->getStartPage()) {
                usedSpace.insert(usedIndex, *usedMemChunk);
                break;
            }
        }
    }

    // add program name to list of programs actively being used by the OS
    programsList.push_front(programName);
    std::cout << "Program " << programName << " added successfully: " << numPages << " page(s) used" << std::endl;
}

// first check if program is present in used memory, if not, exit
// if present, remove chunk from used memory and reclaim in free memory
void OperatingSystem::removeProgram(std::string programName) {
    // make sure the program is present
    if(!containsProgram(programName)) {
        std::cout << "Error, program " << programName << " is not running" << std::endl;
        return;
    }

    // find index in the used memory where the target chunks occurs
    int usedIndex = 0;
    while(usedIndex < usedSpace.getSize() && usedSpace.get(usedIndex).getProgramName() != programName) {
        ++usedIndex;
    }

    // get your target chunk
    // create a new chunk for free memory using reclaimed used memory properties
    Chunk targetChunk = usedSpace.get(usedIndex);
    int numPagesReclaimed = targetChunk.getSize();
    Chunk reclaimedMemory("Free", targetChunk.getStartPage(), targetChunk.getEndPage());

    //insert reclaimed memory back into free memory to maintain order
    int freeIndex;
    bool compareAdjacent;
    // if free space is empty add a head to the free space with the chunk
    if(freeSpace.getSize() == 0) {
        freeSpace.append(reclaimedMemory);
        compareAdjacent = false;
        // if size is 1 compare reclaimed memory to the head (and only) chunk
    } else if(freeSpace.getSize() == 1) {
        // if single chunk start page is greater than reclaimed start page, insert reclaimed chunk at front
        if(freeSpace.get(0).getStartPage() > reclaimedMemory.getStartPage()) {
            freeSpace.insert(0, reclaimedMemory);
            // else add reclaimed chunk after the head node
        } else {
            freeSpace.append(reclaimedMemory);
        }
        compareAdjacent = false;
        // otherwise find best position if the size is greater than 1
    } else {
        for (freeIndex = 1; freeIndex < freeSpace.getSize(); ++freeIndex) {
            if (freeSpace.get(freeIndex).getStartPage() < reclaimedMemory.getStartPage()) {
                freeSpace.insert(freeIndex, reclaimedMemory);
                continue;
            }
        }
        // we didn't already compare adjacent chunks so we must do so now
        compareAdjacent = true;
    }

    // if it's possible/appropriate to compare adjacent chunks in free mem, do so
    if(compareAdjacent) {
        bool merged = false;
        Chunk& originalFree = freeSpace.get(freeIndex);
        Chunk newFree = freeSpace.get(freeIndex - 1);
        // check if end page and start pages of the reclaimed memory and the original free memory chunk line up,
        // if so, merge the two chunk start/end pages to create one contiguous chunk
        if (originalFree.getStartPage() == newFree.getEndPage() + 1) {
            // merge at the start, replace original chunk starting point with new chunk starting point
            originalFree.setStartPage(newFree.getStartPage());
            merged = true;
        } else if (newFree.getEndPage() + 1 == originalFree.getStartPage()) {
            // merge at the end, replace original chunk end point with new chunk end point
            originalFree.setEndPage(newFree.getEndPage());
            merged = true;
        }
        // if we merged two chunks delete the new chunk as it pages were absorbed by the old chunk
        if(merged) {
            freeSpace.remove(freeIndex - 1);
        }
    }

    // remove chunk from used memory
    // remove program name from list of active programs
    usedSpace.remove(usedIndex);
    programsList.remove(programName);
    std::cout << "Program " << programName << " successfully killed, "
              << numPagesReclaimed << " page(s) reclaimed" << std::endl;
}

// a fragment is a set of contiguous pages that are uninterrupted by free space
// therefore the number of fragments is the number of nodes in the free memory
// adjacent pages merge together
int OperatingSystem::getNumFragments() {
    return freeSpace.getSize();
}

// for each page in the OS, print the program name if occupied or else 'Free'
// OS size is 32, print out 4 rows and 8 columns
void OperatingSystem::print() {
    // start at an index of 0
    // if index is in range of used memory and the start page of the chunk at the index of used memory,
    // then print out each page in the chunk using its program name and set index to next start page
    // else increment index and print out 'Free"
    int index = 0;
    while(index < 32) {
        if(index < usedSpace.getSize() && usedSpace.get(index).getStartPage() == index) {
            usedSpace.get(index).print();
            index = usedSpace.get(index).getEndPage() - 1;
        } else {
            ++index;
            std::cout << "Free";
        }
        // start a new line if the 8th column is reached, else print out whitespace
        if (index % 8 == 0) {
            std::cout << std::endl;
        } else {
            std::cout << '\t';
        }
    }
}

// print menu
void displayUserOptionsMenu() {
    std::cout << std::endl;
    std::cout << "1. Add program" << std::endl;
    std::cout << "2. Kill program" << std::endl;
    std::cout << "3. Fragmentation" << std::endl;
    std::cout << "4. Print memory" << std::endl;
    std::cout << "5. Exit" << std::endl;
    std::cout << std::endl;
}

// get menu choice from user, check for bad input and clear bad input
int defineChoice() {
    int userChoice;
    while((std::cout << "Choice - " && !(std::cin >> userChoice)) || userChoice < 1 || userChoice > 5) {
        std::cout << "Error: bad input, enter a number 1 - 5" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        displayUserOptionsMenu();
    }
    return userChoice;
}

// get program name from user, check for and clear bad input
std::string defineProgramName() {
    std::string programName;
    while((std::cout << "Program name - " && !(std::cin >> programName)) || programName == "Free") {
        std::cout << "Error: bad input, program name cannot be 'Free'" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return programName;
}

// get program size from user, check for and clear bad input
int defineProgramSize() {
    int programSize;
    while((std::cout << "Program size (KB) - " && !(std::cin >> programSize)) || programSize <= 0) {
        std::cout << "Error: bad input, enter a size greater than 0" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return programSize;
}

int main(int argc, char* argv[]) {
    // end program if user does not correctly specify algorithm in command line
    std::string algorithmName;
    if (argc == 2) {
        algorithmName = argv[1];
        if (!(algorithmName == "best" || algorithmName == "worst")) {
            std::cout << "Error: must specify algorithm as 'best' or 'worst' thru command line.";
            return 0;
        }
        std::cout << "Using " << algorithmName << " fit algorithm" << std::endl;
    } else {
        std::cout << "Error: must specify algorithm as 'best' or 'worst' thru command line.";
        return 0;
    }

    // initialize operating system with user chosen algorithm 'best' or 'worst' and 32 pages
    OperatingSystem os(algorithmName);

    while(true) {
        displayUserOptionsMenu();
        int userChoice = defineChoice();
        switch (userChoice) {
            case 1: {
                std::string programName = defineProgramName();
                int programSize = defineProgramSize();
                os.addProgram(programName, programSize);
            }
                break;
            case 2: {
                std::string programName = defineProgramName();
                os.removeProgram(programName);
            }
                break;
            case 3: {
                std::cout << "There are " << os.getNumFragments() << " fragment(s)." << std::endl;
            }
                break;
            case 4: {
                os.print();
            }
                break;
            case 5: {
                return 0;
            }
            default: break; // should never happen, we already checked to make sure user input was 1 - 5
        }
    }
}

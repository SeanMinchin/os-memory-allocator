#include "pa2.hpp"
#include <iostream>
#include <utility>
#include <limits>
#include <cmath>

Chunk::Chunk(std::string program_name, int start_page, int endPage): program_name(std::move(program_name)),
                                                                   start_page(start_page), end_page(endPage) {}

const int Chunk::getSize() const {
    return end_page - start_page + 1;
}

const std::string &Chunk::getProgramName() const {
    return program_name;
}

const int& Chunk::getStartPage() const {
    return start_page;
}

const int& Chunk::getEndPage() const {
    return end_page;
}

void Chunk::setStartPage(int start_page) {
    this->start_page = start_page;
}

void Chunk::setEndPage(int end_page) {
    this->end_page = end_page;
}

void Chunk::split(const int& size) {
    start_page += size;
}

std::ostream &operator<<(std::ostream &os, const Chunk &chunk) {
    for(int i = 0; i < chunk.getSize() - 1; ++i) {
        os << chunk.program_name << ',';
    }
    os << chunk.program_name;
    return os;
}

OperatingSystem::OperatingSystem(const std::string& algorithm_chosen) {
    if(algorithm_chosen == "best") {
        algorithm = best;
    } else {
        algorithm = worst;
    }
    free_space.append(Chunk("Free", 1, 32));
    memory_pages.assign(32, "Free");
}

bool OperatingSystem::containsProgram(const std::string& program_name) const {
    // check if OS is already running the program with this name
    return active_programs.count(program_name) != 0;
}

void OperatingSystem::addProgram(std::string program_name, int program_size) {
    // check if program is already running
    if(containsProgram(program_name)) {
        std::cout << "Error, program " << program_name << " already running.";
        return;
    }

    int page_index = -1;
    int current_size;
    switch(algorithm) {
        // find free space chunk with largest size
        case worst: {
            // max size can't be smaller than nothing
            int max_size = 0;
            for (int i = 0; i < free_space.getSize(); ++i) {
                current_size = 4 * free_space.get(i).getSize();
                if (current_size > max_size && current_size >= program_size) {
                    max_size = current_size;
                    page_index = i;
                }
            }
        }
            break;
        // find free space chunk with smallest size that fits program size
        case best: {
            // minimum size can't be larger than the OS size
            int min_size = 129;
            for (int i = 0; i < free_space.getSize(); ++i) {
                current_size = 4 * free_space.get(i).getSize();
                if (current_size < min_size && current_size >= program_size) {
                    min_size = current_size;
                    page_index = i;
                }
            }
        }
            break;
    }

    if(page_index != -1) {
        // total num pages = program size / 4 KB
        auto num_pages = (int) ceil((double) program_size / 4.0);
        // get the target chunk and create a chunk in used memory
        // shift the starting index of the chunk in free memory by the number of pages
        const Chunk &target_chunk = free_space.get(page_index);
        Chunk used_mem_chunk = Chunk(program_name, target_chunk.getStartPage(),
                                     target_chunk.getStartPage() + num_pages - 1);
        free_space.at(page_index).split(num_pages);

        // insert chunk in used space in order by comparing start pages
        switch (used_space.getSize()) {
            // if used memory is empty then append it with a chunk as its head node
            case 0:
                used_space.append(used_mem_chunk);
                break;
            case 1:
                if (used_space.get(0).getStartPage() > used_mem_chunk.getStartPage()) {
                    // if single chunk start page is greater than reclaimed start page, insert reclaimed chunk at front
                    used_space.append(used_mem_chunk);
                } else {
                    // else add the reclaimed chunk after the head node
                    used_space.insert(1, used_mem_chunk);
                }
                break;
            default:
                // else find best position in list
                for (int used_index = 1; used_index < used_space.getSize(); ++used_index) {
                    if (used_space.get(used_index).getStartPage() < used_mem_chunk.getStartPage()) {
                        used_space.insert(used_index + 1, used_mem_chunk);
                        break;
                    }
                }
                break;
        }

        // add program to active programs list
        active_programs.insert(program_name);
        for(int i = used_mem_chunk.getStartPage(); i <= used_mem_chunk.getEndPage(); ++i) {
            memory_pages[i-1] = program_name;
        }
        std::cout << "Program " << program_name << " added successfully: " << num_pages << " page(s) used."
                  << std::endl;
    } else {
        // if no chunk has enough memory
        std::cout << "Error, not enough memory for Program " << program_name;
    }
}

void OperatingSystem::removeProgram(const std::string &program_name) {
    // check if program is active
    if(containsProgram(program_name)) {
        int used_chunk_index = 0;
        while (used_chunk_index < used_space.getSize() &&
               used_space.get(used_chunk_index).getProgramName() != program_name) {
            ++used_chunk_index;
        }

        // get index of target chunk in used memory
        const Chunk& used_mem_chunk = used_space.get(used_chunk_index);
        // remove program name from pages display
        for(int i = used_mem_chunk.getStartPage(); i <= used_mem_chunk.getEndPage(); ++i) {
            memory_pages[i-1] = "Free";
        }
        Chunk reclaimed_memory("Free", used_mem_chunk.getStartPage(), used_mem_chunk.getEndPage());

        if(free_space.isEmpty()) {
            // if there's no free space, simply create the free space head with the chunk
            free_space.append(reclaimed_memory);
        } else if(free_space.getSize() == 1) {
            // if the size is 1 compare the reclaimed memory to the head
            if(free_space.get(0).getStartPage() > reclaimed_memory.getStartPage()) {
                free_space.prepend(reclaimed_memory);
            } else {
                free_space.append(reclaimed_memory);
            }
        } else {
            // otherwise find the best position
            int free_chunk_index;
            for(free_chunk_index = 0; free_chunk_index < free_space.getSize(); ++free_chunk_index) {
                if(free_space.get(free_chunk_index).getStartPage() < reclaimed_memory.getStartPage()) {
                    free_space.insert(free_chunk_index, reclaimed_memory);
                    break;
                }
            }
            bool merged = false;
            Chunk& old_free_chunk = free_space.at(free_chunk_index);
            const Chunk& new_free_chunk = free_space.get(free_chunk_index);
            // check if start page of the original free memory and end page of the reclaimed memory line up
            if(old_free_chunk.getStartPage() == new_free_chunk.getEndPage() + 1) {
                // merge at the start, replacing original chunk starting point with the new chunk starting point
                old_free_chunk.setStartPage(new_free_chunk.getStartPage());
                merged = true;
            } else if (new_free_chunk.getEndPage() + 1 == old_free_chunk.getStartPage()) {
                // merge at the end, replacing original chunk end point with the new chunk end point
                old_free_chunk.setEndPage(new_free_chunk.getEndPage());
                merged = true;
            }
            // if we merged two chunks we can delete the new chunk as the pages were absorbed by the old chunk
            if(merged) {
                free_space.remove(free_chunk_index);
            }
        }

        int num_pages_reclaimed = used_mem_chunk.getSize();
        used_space.remove(used_chunk_index);
        active_programs.erase(program_name);
        std::cout << "Program " << program_name << " successfully killed, " << num_pages_reclaimed <<
                  " page(s) reclaimed." << std::endl;
    } else {
        std::cout << "Error, program " << program_name << " is not running." << std::endl;
    }
}

const int OperatingSystem::getNumFragments() const {
    // a fragment is a set of contiguous pages uninterrupted by free space
    return free_space.getSize();
}

void OperatingSystem::print() const {
    for(unsigned int i = 0; i < memory_pages.size(); ++i) {
        std::cout << memory_pages[i];
        if((i + 1) % 8 == 0) {
            std::cout << std::endl;
        } else {
            std::cout << '\t';
        }
    }
}

std::string defineProgramName() {
    std::string program_name;
    while((std::cout << "Program name - " && !(std::cin >> program_name)) || program_name == "Free") {
        std::cout << "Error: bad input, program name cannot be 'Free'" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return program_name;
}

int defineProgramSize() {
    int program_size;
    while((std::cout << "Program size (KB) - " && !(std::cin >> program_size)) || program_size <= 0) {
        std::cout << "Error: bad input, enter a size greater than 0" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return program_size;
}

std::string defineAlgorithmName() {
    std::string algorithm_name;
    std::cout << "Specify fit algorithm as 'best' or 'worst': ";
    std::cin >> algorithm_name;
    if(algorithm_name != "best" && algorithm_name != "worst") {
        std::cout << "Error: algorithm must be specified as 'best' for best-fit or 'worst' for worst-fit.";
        exit(0);
    }
    std::cout << std::endl << "Using " << algorithm_name << "-fit algorithm" << std::endl;
}

void displayUserOptionsMenu() {
    std::cout << std::endl;
    std::cout << "1. Add program" << std::endl;
    std::cout << "2. Kill program" << std::endl;
    std::cout << "3. Fragmentation" << std::endl;
    std::cout << "4. Print memory" << std::endl;
    std::cout << "5. Exit" << std::endl;
    std::cout << std::endl;
}

int define_choice() {
    int userChoice;
    while((std::cout << "Choice - " && !(std::cin >> userChoice)) || userChoice < 1 || userChoice > 5) {
        std::cout << "Error: bad input, enter a number 1 - 5" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        displayUserOptionsMenu();
    }
    return userChoice;
}

int main() {
    OperatingSystem OS(defineAlgorithmName());
    while(true) {
        displayUserOptionsMenu();
        switch(define_choice()) {
            case 1: {
                std::string program_name = defineProgramName();
                OS.addProgram(program_name, defineProgramSize());
            }
                break;
            case 2:
                OS.removeProgram(defineProgramName());
                break;
            case 3:
                std::cout << "There are " << OS.getNumFragments() << " fragment(s)." << std::endl;
                break;
            case 4:
                OS.print();
                break;
            case 5:
                return 0;
        }
    }
}

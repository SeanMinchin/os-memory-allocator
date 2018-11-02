#ifndef PROJECT_2_PA2_H
#define PROJECT_2_PA2_H

#include <string>
#include <list>

// set of consecutive pages
class Chunk {
private:
    int startPage, endPage;
    std::string programName;
public:
    Chunk() = default;
    Chunk(std::string name, int startBound, int endBound);
    int getStartPage();
    int getEndPage();
    void setStartPage(int page);
    void setEndPage(int page);
    std::string getProgramName();
    int getSize();
    void split(int size);
    void print();
};

struct Node {
    Node() {
        next = nullptr;
    }
    Node* next;
    Chunk data;
};

class LinkedList {
private:
    Node* head;
    int size;
public:
    LinkedList() {
        this->size = 0;
        this->head = nullptr;
    }

    int getSize() {
        return this->size;
    }

    // add data node to end of list
    void append(Chunk value) {
        ++size;
        if(head == nullptr) {
            head = new Node();
            head->data = std::move(value);
        } else {
            Node* temp = head;
            while(temp->next != nullptr) {
                temp = temp->next;
            }
            temp->next = new Node();
            temp->next->data = std::move(value);
        }
    }

    // add data at a location
    void insert(int position, Chunk value) {
        Node* temp = head;
        auto* node = new Node;
        node->data = std::move(value);

        // check if we need to insert at head
        if(position == 0) {
            head = node;
            head->next = temp;
        } else {
            int counter = 0;
            auto* previous = new Node;
            while(counter != position) {
                if(counter == (position - 1)) {
                    previous = temp;
                }
                temp = temp->next;
                ++counter;
            }
            previous->next = node;
            node->next = temp;
        }
        ++size;
    }

    // delete node in linked list at a given position
    bool remove(int position) {
        if(head == nullptr) {
            return false;
        }
        Node* temp = head;
        // check if heads is to be deleted
        if(position == 0) {
            --size;
            head = temp->next;
            return true;
        }
        for(int i = 0; temp != nullptr && i < position - 1; ++i) {
            temp = temp->next;
        }
        // check if position is greater than number of nodes
        if(temp == nullptr || temp->next == nullptr) {
            return false;
        }
        --size;
        temp->next = temp->next->next;
        return true;
    }

    // return a reference to a chunk
    Chunk& get(int position) {
        if(position == 0) {
            return head->data;
        }
        Node* temp = head;
        for(int i = 0; temp != nullptr && i < position -1; ++i) {
            temp = temp->next;
        }
        return temp->next->data;
    }
};

class OperatingSystem {
private:
    std::string algorithmChosen;
    std::list<std::string> programsList;
    bool containsProgram(std::string programName);
    LinkedList freeSpace;
    LinkedList usedSpace;
public:
    explicit OperatingSystem(std::string algorithmChosen);
    void addProgram(std::string programName, int programSize);
    void removeProgram(std::string programName);
    int getNumFragments();
    void print();
};

#endif //PROJECT_2_PA2_H

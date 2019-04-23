#ifndef OS_MEMORY_ALLOCATOR_PA2_HPP
#define OS_MEMORY_ALLOCATOR_PA2_HPP

#include <iostream>
#include <utility>
#include <set>
#include <vector>

template <typename T>
class LinkedList {
private:
    template <typename U>
    struct Node {
        Node* next;
        U data;
        explicit Node(U data): next(nullptr), data(data) {}
    };
    Node<T>* head;
    unsigned int size;
public:
    LinkedList(): head(nullptr), size(0) {}
    ~LinkedList() {
        Node<T>* current = head;
        Node<T>* next = nullptr;
        while(current) {
            next = current->next;
            delete current;
            current = next;
        }
    }
    bool isEmpty() const {
        return !head;
    }
    const unsigned int& getSize() const {
        return size;
    }
    const T& get(const int& index) const {
        if(index >= size) {
            throw std::runtime_error("Node index exceeds size of list");
        }
        if(index == 0) {
            return head->data;
        }
        Node<T>* current = head;
        for(int i = 0; i < index - 1; ++i) {
            current = current->next;
        }
        return current->next->data;
    }
    T& at(const int& index) const {
        if(index >= size) {
            throw std::runtime_error("Node index exceeds size of list");
        }
        if(index == 0) {
            return head->data;
        }
        Node<T>* current = head;
        for(int i = 0; i < index - 1; ++i) {
            current = current->next;
        }
        return current->next->data;
    }
    void append(T value) {
        if(isEmpty()) {
            head = new Node<T>(std::move(value));
        } else {
            Node<T>* current = head;
            while(current->next) {
                current = current->next;
            }
            current->next = new Node<T>(std::move(value));
        }
        ++size;
    }
    void prepend(T value) {
        if(isEmpty()) {
            head = new Node<T>(std::move(value));
        } else {
            Node<T>* current = head;
            head = new Node<T>(std::move(value));
            head->next = current;
        }
        ++size;
    }
    void insert(const int& index, T value) {
        if(index > size) {
            throw std::runtime_error("Node index exceeds size of list");
        }
        if(index == 0) {
            prepend(std::move(value));
        } else if (index == size) {
            append(std::move(value));
        } else {
            Node<T>* current = head;
            Node<T>* previous = nullptr;
            for(int i = 0; i < index; ++i) {
                if(i == index - 1) {
                    previous = current;
                }
                current = current->next;
            }
            previous->next = new Node<T>(std::move(value));
            previous->next->next = current;
            ++size;
        }
    }
    void set(const int& index, T value) {
        if(index >= size) {
            throw std::runtime_error("Node index exceeds size of list");
        }
        if(index == 0) {
            head->data = std::move(value);
        } else {
            Node<T>* current = head;
            for(int i = 0; i < index; ++i) {
                current = current->next;
            }
            current->data = std::move(value);
        }
    }
    void remove(const int& index) {
        if(isEmpty() || index >= size) {
            throw std::runtime_error("Node index exceeds size of list");
        }
        Node<T>* current = head;
        if(index == 0) {
            head = head->next;
            delete current;
        } else {
            Node<T>* previous = nullptr;
            for(int i = 1; i < index + 1; ++i) {
                previous = current;
                current = current->next;
            }
            previous->next = current->next;
            delete current;
        }
        --size;
    }
    void print() const {
        Node<T>* current = head;
        if(isEmpty()) {
            std::cout << "NULL";
        } else {
            while (current) {
                std::cout << current->data;
                if (current->next) {
                    std::cout << "->";
                }
                current = current->next;
            }
        }
        std::cout << std::endl;
    }
};

class Chunk {
private:
    int start_page, end_page;
    std::string program_name;
public:
    Chunk(std::string program_name, int start_page, int endPage);
    const int getSize() const;
    const std::string& getProgramName() const;
    const int& getStartPage() const;
    const int& getEndPage() const;
    void setStartPage(int start_page);
    void setEndPage(int end_page);
    void split(const int& size);
    friend std::ostream& operator<<(std::ostream& os, const Chunk& chunk);
};

class OperatingSystem {
private:
    enum Algorithm {best, worst};
    Algorithm algorithm;
    std::set<std::string> active_programs;
    bool containsProgram(const std::string& program_name) const;
    std::vector<std::string> memory_pages;
    LinkedList<Chunk> free_space;
    LinkedList<Chunk> used_space;
public:
    explicit OperatingSystem(const std::string& algorithm_chosen);
    void addProgram(std::string program_name, int program_size);
    void removeProgram(const std::string& program_name);
    const int getNumFragments() const;
    void print() const;
};

#endif //OS_MEMORY_ALLOCATOR_PA2_HPP

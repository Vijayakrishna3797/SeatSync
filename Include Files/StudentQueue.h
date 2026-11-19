/**
 * @file StudentQueue.h
 * @brief FIFO queue of students, implemented from scratch as a linked list.
 *
 * The queue models the line of students waiting for a seat: the student who
 * registered first is seated first (fair, ordered processing).
 */
#pragma once
#include "Student.h"
#include <vector>

class StudentQueue {
public:
    StudentQueue();
    StudentQueue(const StudentQueue& other);              // deep copy
    StudentQueue(StudentQueue&& other) noexcept;          // steals the nodes
    StudentQueue& operator=(StudentQueue other) noexcept; // copy-and-swap assignment
    ~StudentQueue();

    void enqueue(const Student& s);          // add at the rear            O(1)
    bool dequeue(Student& out);              // remove from the front      O(1), false if empty
    const Student* front() const;            // peek at the front, nullptr if empty
    bool isEmpty() const;
    int size() const;
    void clear();
    std::vector<Student> toVector() const;   // front-to-rear snapshot (for reports)

private:
    struct QNode {
        Student data;
        QNode* next;
    };
    QNode* head_;   // front of the queue
    QNode* tail_;   // rear of the queue
    int size_;

    void swap(StudentQueue& other) noexcept;
};

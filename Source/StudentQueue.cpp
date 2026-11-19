#include "StudentQueue.h"
#include <utility>

StudentQueue::StudentQueue() : head_(nullptr), tail_(nullptr), size_(0) {}

// Deep copy: walk the other list and enqueue each element.
StudentQueue::StudentQueue(const StudentQueue &other) : head_(nullptr), tail_(nullptr), size_(0)
{
    for (QNode *cur = other.head_; cur != nullptr; cur = cur->next)
        enqueue(cur->data);
}

StudentQueue::StudentQueue(StudentQueue &&other) noexcept : head_(nullptr), tail_(nullptr), size_(0)
{
    swap(other);
}

StudentQueue &StudentQueue::operator=(StudentQueue other) noexcept
{
    swap(other); // 'other' is a copy (or moved-from temporary) and cleans up the old nodes
    return *this;
}

StudentQueue::~StudentQueue() { clear(); }

void StudentQueue::swap(StudentQueue &other) noexcept
{
    std::swap(head_, other.head_);
    std::swap(tail_, other.tail_);
    std::swap(size_, other.size_);
}

void StudentQueue::enqueue(const Student &s)
{
    QNode *node = new QNode{s, nullptr};
    if (tail_ == nullptr)
        head_ = tail_ = node; // first element
    else
    {
        tail_->next = node;
        tail_ = node;
    }
    size_++;
}

bool StudentQueue::dequeue(Student &out)
{
    if (head_ == nullptr)
        return false;
    QNode *old = head_;
    out = old->data;
    head_ = head_->next;
    if (head_ == nullptr)
        tail_ = nullptr; // queue became empty
    delete old;
    size_--;
    return true;
}

const Student *StudentQueue::front() const { return head_ ? &head_->data : nullptr; }

bool StudentQueue::isEmpty() const { return size_ == 0; }

int StudentQueue::size() const { return size_; }

void StudentQueue::clear()
{
    Student dummy;
    while (dequeue(dummy))
    {
    }
}

std::vector<Student> StudentQueue::toVector() const
{
    std::vector<Student> out;
    for (QNode *cur = head_; cur != nullptr; cur = cur->next)
        out.push_back(cur->data);
    return out;
}

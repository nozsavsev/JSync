#pragma once

#include <algorithm>
#include <functional>
#include <vector>

template <class T> class VectorEx : public std::vector <T>
{
public:
    using std::vector<T>::vector;

    bool Contains(T val);
    void Rem_All(T val);
    void Rem_If(std::function <bool(T)> fnc);
    void foreach(std::function <void(T &)> fnc);
    void Sort(std::function <bool(T, T)> fnc);

    bool operator==(VectorEx<T> &rhs);
    bool operator!=(VectorEx<T> &rhs);
};

template <class T>
bool VectorEx<T>::Contains(T val)
{
    for ( size_t i = 0; i < this->size(); i++ )
        if ( (*this)[i] == val )
            return true;

    return false;
}

template <class T>
void VectorEx <T>::Rem_All(T val)
{
    this->erase(
        std::remove_if(this->begin(), this->end(), [&] (T &item) -> bool { return (item == val); })
        , this->end());
}

template <class T>
void VectorEx <T>::Rem_If(std::function <bool(T)> fnc) { std::remove_if(this->begin(), this->end(), fnc); }
template <class T>
void VectorEx <T>::foreach(std::function <void(T &)> fnc) { std::for_each(this->begin(), this->end(), fnc); }
template <class T>
void VectorEx <T>::Sort(std::function <bool(T, T)> fnc) { std::sort(this->begin(), this->end(), fnc); }

template <class T>
bool VectorEx <T>::operator==(VectorEx<T> &rhs)
{
    if ( this->size() == rhs.size() )
    {
        return std::equal(this->begin(), this->end(), rhs.begin());
    }

    return false;
}

template <class T>
bool VectorEx <T>::operator!=(VectorEx<T> &rhs)
{
    if ( this->size() == rhs.size() )
    {
        return !std::equal(this->begin(), this->end(), rhs.begin());
    }

    return true;
}
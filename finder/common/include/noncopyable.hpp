#pragma once

namespace finder
{

class noncopyable
{
protected:
    noncopyable() = default;
    ~noncopyable() = default;
private:
    noncopyable(const noncopyable&);
    noncopyable& operator = (const noncopyable&);
};

}
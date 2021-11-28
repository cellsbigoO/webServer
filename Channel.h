#ifndef _CHANNEL_H
#define _CHANNEL_H

#include <functional>

class Channel {
public:
    typedef std::function<void()> Callback;

    Channel();
    ~Channel();

    void SetFd(int fd) {
        fd_ = fd;
    }

    int GetFd() const {
        return fd_;
    }

    void SetEvents(uint32_t events) {
        events_ = events;
    }

    uint32_t GetEvents() const {
        return events_;
    }

    void HandleEvent();

    void setReadHandle(const Callback &cb) {
        readhandler_ = cb;
    }

    void setWriteHandle(const Callback &cb) {
        writehandler_ = cb;
    }

    void setErrorHandle(const Callback &cb) {
        errorhandler_ = cb;
    }

    void SetCloseHandle(const Callback &cb) {
        closehandler_ = cb;
    }
private:
    int fd_;
    uint32_t events_;
    Callback readhandler_;
    Callback writehandler_;
    Callback errorhandler_;
    Callback closehandler_;
};

#endif
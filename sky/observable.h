#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include <algorithm>
#include <functional>
#include <list>

namespace mist
{

template <typename T> using Callback = std::function<void(const T &newValue)>;

template <typename T> class Value;

class ObserverBase
{
};

class Owner
{
public:
    virtual void stopObserving(ObserverBase &s) = 0;
    virtual ~Owner() = default;
};

extern Owner &nullOwner;

// ============================================================================

template <typename T> class Observer : public ObserverBase
{
public:
    Observer() : callback([](T) {}) {}

    Observer(Callback<T> cb) : callback(cb) {}

    ~Observer() { attachedTo.get().stopObserving(*this); }

    bool operator==(const ObserverBase &other) const { return this == &other; }

    void onValue(const T &t) { callback(t); }

    void attach(Value<T> &v) { attachedTo = std::ref(v); }

    void valueDestroyed() { attachedTo = nullOwner; }

    Observer<T> &withCallback(const Callback<T> cb)
    {
        callback = cb;
        return *this;
    }

private:
    Callback<T>                   callback;
    std::reference_wrapper<Owner> attachedTo = nullOwner;
};

// ============================================================================

template <typename T> class Value : public Owner
{
public:
    Value() {}
    Value(const T &initialValue) { value = initialValue; }
    ~Value()
    {
        for (auto f : observers)
            f.get().valueDestroyed();
    }

    T get() const { return value; }

    void set(T newValue)
    {
        value = newValue;
        notifyObservers();
    }

    void change(T newValue)
    {
        if (newValue != value) {
            value = newValue;
            notifyObservers();
        }
    }

    void observe(Observer<T> &observer, const Callback<T> cb)
    {
        observe(observer.withCallback(cb));
    }

    void observe(Observer<T> &observer)
    {
        observers.push_back(observer);
        observer.attach(*this);
        observer.onValue(value);
    }

    void stopObserving(ObserverBase &observer)
    {
        lq::erase_if(observers, ([&](auto &i) {
                         return (i.get() == observer);
                     }));
    }

private:
    T                                              value;
    std::list<std::reference_wrapper<Observer<T>>> observers;

    void notifyObservers() const
    {
        for (auto f : observers)
            f.get().onValue(value);
    }
};

} // namespace mist

#endif

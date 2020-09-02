#include "SCoro.hpp"
#include <thread>
#include <chrono>
#include <iostream>

using system_clock = std::chrono::system_clock;
using time_point = std::chrono::time_point<system_clock>;
using duration = typename system_clock::duration;

static time_point now() noexcept
{
    return system_clock::now();
}

template <typename B>
struct Initial : B
{
    duration wait_time = std::chrono::seconds{1};

    bool Poll() const noexcept
    {
        return true;
    }
};

template <unsigned ms>
struct Delay
{
    template <typename T>
    struct type : T
    {
        using T::T;
    private:
        template <typename B>
        struct RecordTime : B
        {
            using B::B;

            time_point start_time;
            bool Poll() noexcept
            {
                start_time = now();
                return true;
            }
        };

        template <typename B>
        struct WaitForExpiry : B
        {
            using B::B;

            bool Poll() noexcept
            {
                return (now() - B::start_time) > std::chrono::milliseconds{ms};
            }
        };

        using states = SCoro::SCoro<RecordTime, WaitForExpiry>;
        states nested_state;
    public:
        bool Poll() noexcept
        {
            return not nested_state.Poll();
        }
    };
};

template <typename T>
struct PrintPollAttempts : T
{
    using T::T;
private:
    template <typename B>
    struct PrintPlus : B
    {
        static bool Poll() noexcept
        {
            std::printf("/");
            return true;
        }
    };

    template <typename B>
    struct PrintNewline : B
    {
        static bool Poll() noexcept
        {
            std::printf("\\");
            return true;
        }
    };

    using states = SCoro::SCoro<PrintPlus, PrintNewline>;
    states nested_state;
public:
    bool Poll() noexcept
    {
        std::printf("\n");
        return !nested_state.Poll();
    }
};

template <typename T>
struct PrintTag : T
{
    using T::T;

    static bool Poll() noexcept
    {
        std::puts("complete");
        return true;
    }
};


using Coro = SCoro::SCoro
<
    Initial,
    Delay<1000>::type,
    PrintPollAttempts,
    Delay<1000>::type,
    PrintTag
>;

int main()
{
    auto coroutine = Coro{};
    while (true)
    {
        while (coroutine.Poll())
        {
            std::printf("-");
            std::this_thread::sleep_for(std::chrono::milliseconds{10});
        }
        std::printf("\n\n");
        coroutine.Reset();
    }
    return 0;
}
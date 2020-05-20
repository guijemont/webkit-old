/*
 * Copyright (C) 2020 Igalia S.L
 * Copyright (C) 2020 Metrological Group B.V.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once
#include <wtf/DataLog.h>
#include <wtf/Function.h>
#include <wtf/Lock.h>
#include <wtf/Optional.h>
#include <wtf/Seconds.h>
#include <wtf/Stopwatch.h>
#include <wtf/Threading.h>

namespace WTF {

#define FOR_EACH_CODEWATCH_TYPE(macro) \
    macro(Parser) \
    macro(LLInt) \
    macro(JITCompilation) \
    macro(JIT) \
    macro(DFGCompilation) \
    macro(DFG) \
    macro(GC)

enum class CodewatchType : unsigned {
#define CODEWATCH_TYPE_ENUM(type) type,
    FOR_EACH_CODEWATCH_TYPE(CODEWATCH_TYPE_ENUM)
#undef CODEWATCH_TYPE_ENUM
        LastCodewatchType
};

inline const char* codewatchTypeName(CodewatchType t)
{
    switch (t) {
#define CODEWATCH_TYPE_ENUM(type) case CodewatchType::type: return #type;
    FOR_EACH_CODEWATCH_TYPE(CODEWATCH_TYPE_ENUM)
#undef CODEWATCH_TYPE_ENUM
        default:
        RELEASE_ASSERT_NOT_REACHED();
        return nullptr;
    }
}

class Codewatch {
    public:

        static inline void initialize();

        static inline void start(CodewatchType type, const char* origin, void* pc);
        static inline std::optional<CodewatchType> exclusiveStart(CodewatchType type, const char* origin, void* pc, bool failIfNotExclusive=false);
        static inline void stop(CodewatchType type, const char* origin, void* pc);

        /* Note: this method can be called from other threads, whereas other
         * methods will be no-ops from other threads */
        static inline Seconds reset(CodewatchType type);

        static inline std::optional<CodewatchType> stopOthers(CodewatchType type, const char* origin, void* pc);
        static inline void stopAll(const char* origin, void *pc);

        static inline bool isAnyActive();

        static inline void logHeader();
        static inline void resetAllAndLog();

    private:
        Codewatch(CodewatchType type)
        : m_stopWatch(Stopwatch::create())
        , m_lastPc(0)
        , m_type(type)
        { }

        static inline Codewatch& get(CodewatchType type) {
            RELEASE_ASSERT(type < CodewatchType::LastCodewatchType);
            RELEASE_ASSERT(m_initialized);
            return *m_codewatches[static_cast<size_t>(type)];
        }

        inline void startImpl(const char *origin, void* pc) {
            //dataLogF("[%p] start(%d) %s %p\n", &WTF::Thread::current(), static_cast<int>(T), origin, pc);
            UNUSED_PARAM(origin);
            m_lastPc = pc;
            if (!isActive())
                m_stopWatch->start();
        };

        inline void stopImpl(const char* origin, void* pc) {
            //dataLogF("[%p] stop(%d) %s %p\n", &WTF::Thread::current(), static_cast<int>(T), origin, pc);
            UNUSED_PARAM(origin);
            UNUSED_PARAM(pc);
            if (isActive()) {
                m_stopWatch->stop();
            }
        }

        inline Seconds resetImpl() {
            Seconds elapsedTimeBeforeReset;
            //dataLogF("[%p] Resetting Codewatch<%d>\n", &WTF::Thread::current(), static_cast<int>(T));
            bool wasRunning = isActive();
            elapsedTimeBeforeReset = elapsedTime();
            m_stopWatch->reset();
            if (wasRunning)
                startImpl(WTF_PRETTY_FUNCTION, m_lastPc);
            return elapsedTimeBeforeReset;
        }

        static inline bool isInOriginalThread() {
            return (m_thread.get() == &Thread::current());
        }

        inline Seconds elapsedTime() { return m_stopWatch->elapsedTime(); }
        inline bool isActive() { return m_stopWatch->isActive(); }


        static Vector<Codewatch*> m_codewatches;
        static bool m_initialized;
        static RefPtr<Thread> m_thread;
        static Lock m_lock;
        Ref<Stopwatch> m_stopWatch;
        void* m_lastPc;
        CodewatchType m_type;

};

inline void Codewatch::initialize() {
    RELEASE_ASSERT(!m_initialized);
    //m_codewatches = Vector<Codewatch*>(static_cast<size_t>(CodewatchType::LastCodewatchType), nullptr);
#define CODEWATCH_TYPE_ENUM(type) \
    m_codewatches[static_cast<size_t>(CodewatchType::type)] = new Codewatch(CodewatchType::type);
    FOR_EACH_CODEWATCH_TYPE(CODEWATCH_TYPE_ENUM)
#undef CODEWATCH_TYPE_ENUM
        m_thread = &Thread::current();
    m_initialized = true;
}

inline void Codewatch::start(CodewatchType type, const char* origin, void* pc) {
    RELEASE_ASSERT(m_initialized);
    Codewatch& codewatch = get(type);
    if (codewatch.isActive())
        return;
    RELEASE_ASSERT(!isAnyActive());
    bool isExpectedThread = isInOriginalThread();
    //dataLogF("Codewatch.start;%p;%d;%s;%p\n", &Thread::current(), isExpectedThread, origin, pc);
    if (!isExpectedThread) {
        return;
    }
    m_lock.lock();
    codewatch.startImpl(origin, pc);
    m_lock.unlock();
}

inline void Codewatch::stop(CodewatchType type, const char* origin, void* pc) {
    RELEASE_ASSERT(m_initialized);
    Codewatch& codewatch = get(type);
    bool isExpectedThread = isInOriginalThread();
    //dataLogF("Codewatch.stop;%p;%d;%s;%p\n", &Thread::current(), isExpectedThread, origin, pc);
    if (!isExpectedThread) {
        return;
    }
    m_lock.lock();
    codewatch.stopImpl(origin, pc);
    m_lock.unlock();
}


/* Note: this method can be called from other threads */
inline Seconds Codewatch::reset(CodewatchType type) {
    RELEASE_ASSERT(m_initialized);
    Seconds retval;
    Codewatch& codewatch = get(type);
    m_lock.lock();
    retval = codewatch.resetImpl();
    m_lock.unlock();
    return retval;
}

inline std::optional<CodewatchType> Codewatch::stopOthers(CodewatchType type, const char* origin, void* pc) {
    RELEASE_ASSERT(m_initialized);
    std::optional<CodewatchType> ret = std::nullopt;
#define CODEWATCH_TYPE_ENUM(_type) \
    if (type != CodewatchType::_type && get(CodewatchType::_type).isActive()){ \
        RELEASE_ASSERT(!ret); /* no more than one Codewatch can be running at once */ \
        ret = std::optional<CodewatchType>({CodewatchType::_type}); \
        stop(CodewatchType::_type, origin, pc); \
    }
    FOR_EACH_CODEWATCH_TYPE(CODEWATCH_TYPE_ENUM)
#undef CODEWATCH_TYPE_ENUM
    return ret;
}

inline void Codewatch::stopAll(const char* origin, void *pc) {
    RELEASE_ASSERT(m_initialized);
#define CODEWATCH_TYPE_ENUM(type) \
    stop(CodewatchType::type, origin, pc);
    FOR_EACH_CODEWATCH_TYPE(CODEWATCH_TYPE_ENUM)
#undef CODEWATCH_TYPE_ENUM
}

inline bool Codewatch::isAnyActive() {
    RELEASE_ASSERT(m_initialized);
#define CODEWATCH_TYPE_ENUM(type) \
    if (get(CodewatchType::type).isActive()) return true;
    FOR_EACH_CODEWATCH_TYPE(CODEWATCH_TYPE_ENUM)
#undef CODEWATCH_TYPE_ENUM
        return false;
}

inline std::optional<CodewatchType> Codewatch::exclusiveStart(CodewatchType type, const char* origin, void* pc, bool failIfNotExclusive) {
    RELEASE_ASSERT(m_initialized);
    if (!isInOriginalThread())
        return std::nullopt;
    auto ret = stopOthers(type, origin, pc);
    if (failIfNotExclusive)
        RELEASE_ASSERT(!ret);
    start(type, origin, pc);
    return ret;
}

inline void Codewatch::logHeader() {
    RELEASE_ASSERT(m_initialized);
#define CODEWATCH_TYPE_ENUM(type) \
    " "#type,
    dataLogLn("Frame TimeStamp",
            FOR_EACH_CODEWATCH_TYPE(CODEWATCH_TYPE_ENUM)
            "");
#undef CODEWATCH_TYPE_ENUM
}

inline void Codewatch::resetAllAndLog() {
    RELEASE_ASSERT(m_initialized);
    auto now = MonotonicTime::now();
#define CODEWATCH_TYPE_ENUM(type) \
    Seconds timeFor##type = Codewatch::reset(CodewatchType::type);
    FOR_EACH_CODEWATCH_TYPE(CODEWATCH_TYPE_ENUM)
#undef CODEWATCH_TYPE_ENUM

#define CODEWATCH_TYPE_ENUM(type) \
        " ", timeFor##type.value() * 1000.0,
        dataLogLn("Frame ", now.secondsSinceEpoch().value(),
                FOR_EACH_CODEWATCH_TYPE(CODEWATCH_TYPE_ENUM)
                "");
#undef CODEWATCH_TYPE_ENUM
}

}

using WTF::CodewatchType;
using WTF::Codewatch;

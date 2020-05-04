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
#include <wtf/Lock.h>
#include <wtf/Stopwatch.h>
#include <wtf/Threading.h>

namespace WTF {

enum class CodewatchType { LLInt, JIT, DFG };

template <CodewatchType T>
class Codewatch {
    public:
        static inline Codewatch& getCodewatch() {
            if (!m_codewatch) {
                m_codewatch = new Codewatch<T>();
                dataLogF("[%p] Creating new Codewatch<%d>: %p\n", &Thread::current(), static_cast<int>(T), m_codewatch);
            }
            return *m_codewatch;
        }

        inline Seconds reset() {
            Seconds elapsedTimeBeforeReset;
            m_lock.lock();
            //dataLogF("[%p] Resetting Codewatch<%d>\n", &WTF::Thread::current(), static_cast<int>(T));
            bool wasRunning = isActive();
            elapsedTimeBeforeReset = elapsedTime();
            m_stopWatch->reset();
            if (wasRunning)
                startImpl(WTF_PRETTY_FUNCTION, m_lastPc);
            m_lock.unlock();
            return elapsedTimeBeforeReset;
        }

        inline void start(const char *origin, void* pc) {
            bool isExpectedThread = (m_thread.get() == &Thread::current());
            //dataLogF("Codewatch.start;%p;%d;%s;%p\n", &Thread::current(), isExpectedThread, origin, pc);
            if (!isExpectedThread) {
                return;
            }
            m_lock.lock();
            startImpl(origin, pc);
            m_lock.unlock();
        }

        inline void stop(const char *origin, void* pc) {
            bool isExpectedThread = (m_thread.get() == &Thread::current());
            //dataLogF("Codewatch.stop;%p;%d;%s;%p\n", &Thread::current(), isExpectedThread, origin, pc);
            if (!isExpectedThread) {
                return;
            }
            m_lock.lock();
            //dataLogF("[%p] stop(%d) %s %p\n", &WTF::Thread::current(), static_cast<int>(T), origin, pc);
            UNUSED_PARAM(origin);
            UNUSED_PARAM(pc);
            if (isActive()) {
                m_stopWatch->stop();
            }
            m_lock.unlock();
        }

        inline Seconds elapsedTime() { return m_stopWatch->elapsedTime(); }
        inline bool isActive() { return m_stopWatch->isActive(); }

    private:
        Codewatch() : m_stopWatch(Stopwatch::create()), m_lastPc(0), m_thread(&Thread::current()) { }

        inline void startImpl(const char *origin, void* pc) {
            //dataLogF("[%p] start(%d) %s %p\n", &WTF::Thread::current(), static_cast<int>(T), origin, pc);
            UNUSED_PARAM(origin);
            m_lastPc = pc;
            if (!isActive())
                m_stopWatch->start();
        };

        Ref<Stopwatch> m_stopWatch;
        static Codewatch<T>* m_codewatch;
        void* m_lastPc;
        Lock m_lock;
        RefPtr<Thread> m_thread;

};

template <CodewatchType T>
Codewatch<T>* Codewatch<T>::m_codewatch = 0;

}

using WTF::CodewatchType;
using WTF::Codewatch;

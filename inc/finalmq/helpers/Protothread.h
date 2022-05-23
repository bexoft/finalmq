// MIT License
//
// Copyright (c) 2018 Ben Hoyt
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Protothread class and macros for lightweight, stackless threads in C++.
//
// This was "ported" to C++ from Adam Dunkels' protothreads C library at:
//     http://www.sics.se/~adam/pt/
//
// Originally ported for use by Hamilton Jet (www.hamiltonjet.co.nz) by
// Ben Hoyt, but stripped down for public release. See his blog entry about
// it for more information:
//     http://blog.brush.co.nz/2008/07/protothreads/
//
// Visual Studio users: There's a quirk with VS where it defines __LINE__
// as a non-constant when you've got a project's Debug Information Format
// set to "Program Database for Edit and Continue (/ZI)" -- the default.
// To fix, just go to the project's Properties, Configuration Properties,
// C/C++, General, Debug Information Format, and change it to "Program
// Database (/Zi)".
//
// Protothread.h is licensed under an MIT license (see LICENSE.txt).
// Adam's original library was licensed under a similar, BSD-style
// license, which lives at: http://dunkels.com/adam/pt/license.html
//

#ifndef __PROTOTHREAD_H__
#define __PROTOTHREAD_H__

// A lightweight, stackless thread. Override the Run() method and use
// the PT_* macros to do work of the thread.
//
// A simple example
// ----------------
// class LEDFlasher : public Protothread
// {
// public:
//     virtual bool Run();
//
// private:
//     ExpiryTimer _timer;
//     uintf _i;
// };
//
// bool LEDFlasher::Run()
// {
//     PT_BEGIN();
//
//     for (_i = 0; _i < 10; _i++)
//     {
//         SetLED(true);
//         _timer.Start(250);
//         PT_WAIT_UNTIL(_timer.Expired());
//
//         SetLED(false);
//         _timer.Start(750);
//         PT_WAIT_UNTIL(_timer.Expired());
//     }
//
//     PT_END();
// }
//
class Protothread
{
public:
    // Construct a new protothread that will start from the beginning
    // of its Run() function.
    Protothread() : _ptLine(0) { }

    // Restart protothread.
    void Restart() { _ptLine = 0; }

    // Stop the protothread from running. Happens automatically at PT_END.
    // Note: this differs from the Dunkels' original protothread behaviour
    // (his restart automatically, which is usually not what you want).
    void Stop() { _ptLine = LineNumberInvalid; }

    // Return true if the protothread is running or waiting, false if it has
    // ended or exited.
    bool IsRunning() { return _ptLine != LineNumberInvalid; }

    // Run next part of protothread or return immediately if it's still
    // waiting. Return true if protothread is still running, false if it
    // has finished. Implement this method in your Protothread subclass.
    virtual bool Run() = 0;

    // Used to store a protothread's position (what Dunkels calls a
    // "local continuation").
    typedef unsigned short LineNumber;

    // An invalid line number, used to mark the protothread has ended.
    static const LineNumber LineNumberInvalid = (LineNumber)(-1);

    // Stores the protothread's position (by storing the line number of
    // the last PT_WAIT, which is then switched on at the next Run).
    LineNumber _ptLine;
};

// Declare start of protothread (use at start of Run() implementation).
#define PT_BEGIN(protothread) bool ptYielded = true; (void) ptYielded; switch (protothread._ptLine) { case 0:

// Stop protothread and end it (use at end of Run() implementation).
#define PT_END(protothread) default: ; } protothread.Stop(); return false;

// Cause protothread to wait until given condition is true.
#define PT_WAIT_UNTIL(protothread, condition) \
    do { protothread._ptLine = __LINE__; case __LINE__: \
    if (!(condition)) return true; } while (0)

// Cause protothread to wait while given condition is true.
#define PT_WAIT_WHILE(protothread, condition) PT_WAIT_UNTIL(!(protothread, condition))

// Cause protothread to wait until given child protothread completes.
#define PT_WAIT_THREAD(child) PT_WAIT_WHILE((child).Run())

// Restart and spawn given child protothread and wait until it completes.
#define PT_SPAWN(child) \
    do { (child).Restart(); PT_WAIT_THREAD(child); } while (0)

// Restart protothread's execution at its PT_BEGIN.
#define PT_RESTART(protothread) do { protothread.Restart(); return true; } while (0)

// Stop and exit from protothread.
#define PT_EXIT() do { Stop(); return false; } while (0)

// Yield protothread till next call to its Run().
#define PT_YIELD(protothread) \
    do { ptYielded = false; protothread._ptLine = __LINE__; case __LINE__: \
    if (!ptYielded) return true; } while (0)

// Yield protothread until given condition is true.
#define PT_YIELD_UNTIL(protothread, condition) \
    do { ptYielded = false; protothread._ptLine = __LINE__; case __LINE__: \
    if (!ptYielded || !(condition)) return true; } while (0)

#endif // __PROTOTHREAD_H__

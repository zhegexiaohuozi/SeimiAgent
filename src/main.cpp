/*
   Copyright 2016 Wang Haomiao<et.tw@163.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 */
#include <QApplication>
#include "SeimiAgent.h"
#include "crashdump.h"

int main(int argc, char *argv[])
{
    try {
        init_crash_handler();
        SeimiAgent *agent = SeimiAgent::instance();
        return agent->run(argc,argv);

    } catch (std::bad_alloc) {
        fputs("Memory exhausted.\n", stderr);
        fflush(stderr);
        return 1;

    } catch (std::exception& e) {
        fputs("Uncaught C++ exception: ", stderr);
        fputs(e.what(), stderr);
        putc('\n', stderr);
        print_crash_message();
        return 1;

    } catch (...) {
        fputs("Uncaught nonstandard exception.\n", stderr);
        print_crash_message();
        return 1;
    }

}


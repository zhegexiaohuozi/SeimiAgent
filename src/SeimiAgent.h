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
#ifndef SEIMIAGENT_H
#define SEIMIAGENT_H
#include <QObject>
#include <cookiejar.h>

class SeimiAgent: public QObject
{
    Q_OBJECT
private:
    SeimiAgent(QObject *parent = 0);
public:
    static SeimiAgent* instance();
    int run(int argc, char *argv[]);
    CookieJar* getCookieJar();
    void cleanAllcookies();
private:
    CookieJar* _defaultCookieJar;
};

#endif // SEIMIAGENT_H

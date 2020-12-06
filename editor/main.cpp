/*
    Stride is licensed under the terms of the 3-clause BSD license.

    Copyright (C) 2017. The Regents of the University of California.
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Authors: Andres Cabrera and Joseph Tilbian
*/

#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <clocale>

#include "projectwindow.h"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context,
                     const QString &msg) {
  QByteArray localMsg = msg.toLocal8Bit();
  const char *file = context.file ? context.file : "";
  const char *function = context.function ? context.function : "";
  if (msg.contains("sing QCharRef with an index pointing outside the valid "
                   "range of a QString")) {
    int a = 1;
    a = a + 1;
  }

  switch (type) {
    case QtDebugMsg:
      fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file,
              context.line, function);
      break;
    case QtInfoMsg:
      fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file,
              context.line, function);
      break;
    case QtWarningMsg:
      fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file,
              context.line, function);
      break;
    case QtCriticalMsg:
      fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file,
              context.line, function);
      break;
    case QtFatalMsg:
      fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file,
              context.line, function);
      break;
  }
}

int main(int argc, char *argv[]) {
  qInstallMessageHandler(myMessageOutput);
  qSetMessagePattern(
      "[%{time yyyy-MM-dd "
      "h:mm:ss.zzz}%{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{"
      "endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}][%{file}:%{line} "
      "%{function}] %{message}");
  char *lc;
  if (!(lc = setlocale(LC_ALL, nullptr))) {
    qDebug() << "Error setting locale.";
  }
  qDebug() << "Using locale " << lc;

  // TODO move this crash marker to the temp directory?
  bool previousRunCrashed = QFile::exists(".crashmarker");
  QFile crashMarker(".crashmarker");
  crashMarker.open(QFile::WriteOnly);
  crashMarker.write("oops");
  crashMarker.close();

  QApplication a(argc, argv);

  if (previousRunCrashed) {
    QMessageBox::StandardButton choice = QMessageBox::question(
        nullptr, "Reset?", "Stride IDE crashed. Would you like to reset it?");
    if (choice == QMessageBox::No) {
      previousRunCrashed = false;
    }
  }

  ProjectWindow w;
  a.installEventFilter(&w);  // Pass events from a to w
  w.initialize(previousRunCrashed);
  w.show();

  int ret = a.exec();
  if (ret == 0) {
    QFile::remove(".crashmarker");
  }

  return ret;
}

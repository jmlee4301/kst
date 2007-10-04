/***************************************************************************
 *                                                                         *
 *   copyright : (C) 2007 The University of Toronto                        *
 *   copyright : (C) 2005  University of British Columbia                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MATRIXDEFAULTS_H
#define MATRIXDEFAULTS_H

#include <qstring.h>
#include "kst_export.h"

class QSettings;

namespace Kst {

class KST_EXPORT MatrixDefaults {
  public:
    // constructor
    MatrixDefaults();
    // get last used settings and store them in this MatrixDefaults
    void sync();
    void readConfig(QSettings *config);
    void writeConfig(QSettings *config);
    
    const QString& dataSource() const;
    int xStart() const;
    bool xCountFromEnd() const;
    int yStart() const;
    bool yCountFromEnd() const;
    int xNumSteps() const;
    bool xReadToEnd() const;
    int yNumSteps() const;
    bool yReadToEnd() const;
    bool doSkip() const;
    bool doAverage() const;
    int skip() const;

  private:
    QString _dataSource;
    int _xStart; // -1 to count from end
    int _yStart; // -1 to count from end
    int _xNumSteps; // -1 to read to end
    int _yNumSteps; // -1 to read to end
    bool _doSkip;
    bool _doAve;
    int _skip;
};

  extern KST_EXPORT MatrixDefaults matrixDefaults;

}

#endif
// vim: ts=2 sw=2 et
#include <QApplication>
#include "neoadaedit.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    NeoAdaEdit edt;

    edt.restoreState();
    edt.show();

    return app.exec();
}

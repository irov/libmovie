#include "Viewer.h"

int main(int argc, char** argv)
{
    Viewer viewer;
    if( viewer.Initialize( argc, argv ) == false )
    {
        return -1;
    }

    viewer.Loop();
    viewer.Finalize();

    return 0;
}

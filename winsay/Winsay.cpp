//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
USERES("Winsay.res");
USEFORM("Main.cpp", MainForm);
USEFORM("AboutFrm.cpp", FrmAbout);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
        try
        {
                 Application->Initialize();
                 Application->Title = "Winsay";
                 Application->CreateForm(__classid(TMainForm), &MainForm);
                 Application->CreateForm(__classid(TFrmAbout), &FrmAbout);
                 Application->Run();
        }
        catch (Exception &exception)
        {
                 Application->ShowException(&exception);
        }
        return 0;
}
//---------------------------------------------------------------------------

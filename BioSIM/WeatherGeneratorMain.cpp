//***********************************************************************
// Example program to generate weather and call model execution
//
//***********************************************************************
//03/10/2025    Rémi Saint-Amant	Cross-platform version



#include <cstdio>
#include <iostream>

#include <boost/timer/timer.hpp>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>

#include "Basic/UtilStd.h"
#include "Geomatic/GDAL.h"
#include "BioSIM_API.h"
#include <boost/dll/shared_library.hpp>

//"ModelsPath=E:/ProjectCP/WBSFTools/BioSIM/bin/Debugx64/Models/&shore=G:/Travaux/BioSIM_API/Layers/Shore.ann&DEM=G:/Travaux/BioSIM_API/DEM/Monde 30s(SRTM30).tif"  "Normals=G:/Travaux/BioSIM_API/Weather/Normals/World 1991-2020.NormalsDB&Daily=G:/Travaux/BioSIM_API/Weather/Daily/Demo 2008-2010.DailyDB"




using namespace std;
using namespace WBSF;


//***********************************************************************
//
//	Main
//
//***********************************************************************
int main(int argc, char* argv[])
{
    boost::timer::cpu_timer timer;
    timer.start();


    GDALAllRegister();

    string exe_path = argv[0];


    //Initialized static data
    string globa_data = argv[1];
    string options = argv[2];



    cout << "Initialize Weather Generator" << endl;

    CBioSIM_API_GlobalData global;
    string msg = global.InitGlobalData(globa_data);
    cout << msg << endl;

    if (msg == "Success")
    {
        cout << "Time to initialize global data: " << timer.elapsed().wall / 1e9 << " s" << endl << endl;

        cout << "Initialize Weather Generator" << endl;
        timer.start();
        CWeatherGeneratorAPI WG("WG1");
        msg = WG.Initialize(options);
        cout << msg << endl;


        if (msg == "Success")
        {
            cout << "Time to initialize Weather Generator: " << timer.elapsed().wall / 1e9 << " s" << endl << endl;

            timer.start();
            CTeleIO WGout = WG.Generate("Compress=0&Variables=T&ID=1&Name=Logan&Latitude=41.73333333&Longitude=-111.8&Elevation=120&First_year=2008&Last_year=2010&Replications=1");
            msg = WGout.m_msg;
            cout << msg << endl;


            if (msg == "Success")
            {
                cout << "Time to generate weather: " << timer.elapsed().wall / 1e9 << " s" << endl << endl;
                cout << "Initialize DegreeDay model" << endl;

                timer.start();
                CModelExecutionAPI model("Model1");
                msg = model.Initialize("Model=DegreeDay(Annual).mdl");
                cout << msg << endl;



                if (msg == "Success")
                {
                    cout << "Time to initialize DegreeDay model: " << timer.elapsed().wall / 1e9 << " s" << endl << endl;
                    string variables = model.GetWeatherVariablesNeeded();
                    string parameters = model.GetDefaultParameters();
                    bool compress = false;
                    string compress_str = compress ? "1" : "0";


                    //&Elevation=2800
                    //example of extracting weather from
                    cout << "Generate weather for 2008-2010, 1 replications" << endl;
                    timer.start();
                    CTeleIO WGout = WG.Generate("Compress=" + compress_str + "&Variables=" + variables + "&ID=1&Name=Logan&Latitude=41.73333333&Longitude=-111.8&Elevation=120&First_year=2008&Last_year=2010&Replications=1");
                    cout << WGout.m_msg << endl;

                    if (WGout.m_msg == "Success")
                    {
                        cout << "Time to generate weather: " << timer.elapsed().wall / 1e9 << " s" << endl << endl;
                        cout << "Execute DegreeDay Model" << endl;
                        CTeleIO modelOut = model.Execute("Compress=" + compress_str, WGout);
                        cout << modelOut.m_msg << endl;
                        cout << modelOut.m_data << endl;


                        if (WGout.m_msg == "Success")
                        {
                            cout << "Time to run DD model: " << timer.elapsed().wall / 1e9 << " s" << endl << endl;
                            cout << endl << endl << "Second generation" << endl;

                            timer.start();
                            WGout = WG.Generate("Compress=" + compress_str + "&Variables=" + variables + "&ID=1&Name=Logan&Latitude=41.73333333&Longitude=-111.8&Elevation=120&First_year=2008&Last_year=2010&Replications=1");

                            if (WGout.m_msg == "Success")
                            {
                                cout << "Time to generate weather (second time): " << timer.elapsed().wall / 1e9 << " s" << endl << endl;

                                timer.start();
                                modelOut = model.Execute("Compress=" + compress_str, WGout);
                                cout << modelOut.m_data << endl;
                                if (WGout.m_msg == "Success")
                                {
                                    cout << "Time to run DD model (second time): " << timer.elapsed().wall / 1e9 << " s" << endl << endl;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    //Create a mergeImages object
    //CWeatherGenerationApp WeatherGenerator;

    //ERMsg msg = WeatherGenerator.m_options.ParseOption(argc, argv);

    //if (!WeatherGenerator.m_options.m_bQuiet)
    //	cout << WeatherGenerator.GetDescription() << endl;


    //if (msg)
    //	msg = WeatherGenerator.Execute();

    //if (!msg)
    //{
    //	PrintMessage(msg);
    //	return -1;
    //}


    timer.stop();
    cout << endl << "Total time: " << BioSIMAPI_SecondToDHMS(timer.elapsed().wall / 1e9) << endl;


    getc(stdin);
    //getch();

    return 0;
}

#include <iostream>
#include <fstream>
#include <TSystem.h>
#include <TImage.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TEllipse.h>
#include <TLine.h>
#include <TMath.h>
#include <TF1.h>
#include <TText.h>
#include <dirent.h>
#include <sys/stat.h>
#include <TColor.h>
#include <TPaletteAxis.h>
#include <TDatime.h>
#include <sys/stat.h>
#include <ctime>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <TLatex.h>    
#include <TPaveStats.h>
//#include <Magick++.h>

using namespace std;

std::string getFile(const std::string &dirPath) {
    DIR *dir;
    struct dirent *ent;
    struct stat statbuf;
    std::string latestFile;
    time_t latestTime = 0;

    if ((dir = opendir(dirPath.c_str())) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            std::string filePath = dirPath + "/" + ent->d_name;
            stat(filePath.c_str(), &statbuf);
            if (S_ISREG(statbuf.st_mode)) {
                if (statbuf.st_mtime > latestTime) {
                    latestTime = statbuf.st_mtime;
                    latestFile = filePath;
                }
            }
        }
        cout<<"Reading>>>>"<<latestFile<<endl;
        closedir(dir);
    } else {
        std::cerr << "Could not open directory: " << dirPath << std::endl;
    }
    return latestFile;
}

void deleteFile(const char* imagePath){
    if(FILE *file = fopen(imagePath,"r")){
        fclose(file);
        if(std::remove(imagePath)==0){
            std::cout<<"File deleted: "<<imagePath<<std::endl;
        } else{
            std::cerr<<"Error deleting file"<<std::strerror(errno)<<std::endl;
        }
    }else{
        std::cerr << "FILE does not exist or cannot be accessed: " << imagePath << std::endl;
    }
}

void anaROI(TH2F *h3, double xMin, double xMax, double yMin, double yMax, bool drawLatex) {
    TH2F *h3Clone = (TH2F*)h3->Clone("h3Clone");

    h3Clone->GetXaxis()->SetRangeUser(-10, 10);
    h3Clone->GetYaxis()->SetRangeUser(-10, 10);
    TCanvas *cROI = new TCanvas("cROI", "Region of Interest", 4096/3, 3000/3);
    gStyle->SetOptStat(0);
    h3Clone->Draw("COLZ");
    //cROI->SaveAs("ROI.png");

    //gStyle->SetOptStat(1110);
    //gStyle->SetOptFit(111);
    gStyle->SetOptStat(0000);
    gStyle->SetOptFit(000);
    gStyle->SetStatStyle(4001);

    TCanvas *cProjX = new TCanvas("cProjX", "X Profile", 4096/3, 3000/3);
    TH1D *projX = h3Clone->ProjectionX("projX");
    cProjX->SetLeftMargin(0.15);
    projX->SetTitle("");
    projX->GetXaxis()->SetLabelSize(0.05);
    projX->GetXaxis()->SetTitleSize(0.05);
    projX->GetXaxis()->SetTickLength(0.03);
    projX->GetXaxis()->SetRangeUser(-10, 10);
    projX->GetXaxis()->SetTitle("X (mm)");
    projX->GetYaxis()->SetTitle("Counts");
    projX->GetYaxis()->SetTitleSize(0.05);
    projX->GetYaxis()->SetTitleOffset(1.4);
    projX->GetYaxis()->SetLabelSize(0.05);
    projX->GetXaxis()->SetNdivisions(510); // 5 major ticks, 10 minor ticks
    projX->SetFillColor(38);
    projX->Draw();

    TF1 *gausX = new TF1("gausX", "gaus", -10, 10);
    //if (projX->Integral() > 3E6) projX->Fit(gausX, "Q");
    projX->Fit(gausX, "Q");
    double meanX = gausX->GetParameter(1);
    double sigmaX = gausX->GetParameter(2);
    TPaveStats *statsX = (TPaveStats*)projX->FindObject("stats");
    if (statsX) {
        statsX->SetX1NDC(0.70); 
        statsX->SetX2NDC(1.0); 
        statsX->SetY1NDC(0.65); 
        statsX->SetY2NDC(1.0); 
        statsX->SetTextColor(kBlue+2);
        statsX->SetTextSize(0.03);
    }
    //else {cout<<"no stats"<<endl;}

    TLatex *latexXtitle = new TLatex(0.43, 0.91, Form("X profile"));
    latexXtitle->SetNDC();
    //latexXtitle->SetTextColor(kRed);
    latexXtitle->SetTextSize(0.06);  
    latexXtitle->Draw();

    if (drawLatex) {
        //TLatex *latexX = new TLatex(xMax * 0.77, projX->GetMaximum() * 0.6,
        TLatex *latexX = new TLatex(0.65, 0.91,
                Form("%.2f#pm%.2f (mm)", meanX, sigmaX));
        latexX->SetNDC();
        latexX->SetTextColor(kRed);
        latexX->SetTextSize(0.05);
        latexX->Draw();
    }

    cProjX->Update();


    cProjX->Modified();
    cProjX->Update();
    cProjX->SaveAs("ProjX.png");

    TCanvas *cProjY = new TCanvas("cProjY", "Y Profile", 4096/3, 3000/3);
    TH1D *projY = h3Clone->ProjectionY("projY");
    cProjY->SetLeftMargin(0.15);
    projY->SetTitle("");
    projY->GetXaxis()->SetLabelSize(0.05);
    projY->GetXaxis()->SetTitleSize(0.05);
    projY->GetXaxis()->SetTickLength(0.03);
    projY->GetXaxis()->SetRangeUser(-10, 10);
    projY->GetXaxis()->SetTitle("Y (mm)");
    projY->GetYaxis()->SetTitle("Counts");
    projY->GetYaxis()->SetLabelSize(0.05);
    projY->GetYaxis()->SetTitleOffset(1.4);
    projY->GetYaxis()->SetTitleSize(0.05);
    projY->GetXaxis()->SetNdivisions(510);
    projY->SetFillColor(38);
    projY->Draw();

    TF1 *gausY = new TF1("gausY", "gaus", -10, 10);
    //if (projY->Integral() > 3E6) projY->Fit(gausY, "Q");
    TPaveStats *statsY = (TPaveStats*)projY->FindObject("stats");
    if (statsY) {
        statsY->SetX1NDC(0.70); 
        statsY->SetX2NDC(1.0); 
        statsY->SetY1NDC(0.65); 
        statsY->SetY2NDC(1.0); 
        statsY->SetTextColor(kBlue+2);
        statsY->SetTextSize(0.03);
    }

    TLatex *latexYtitle = new TLatex(0.43, 0.91, Form("Y profile"));
    latexYtitle->SetNDC();
    //latexYtitle->SetTextColor(kRed);
    latexYtitle->SetTextSize(0.06);  
    latexYtitle->Draw();

    projY->Fit(gausY, "Q");
    double meanY = gausY->GetParameter(1);
    double sigmaY = gausY->GetParameter(2);
    if (drawLatex) {
        //TLatex *latexY = new TLatex(xMax * 0.77, projY->GetMaximum() * 0.6,
        TLatex *latexY = new TLatex(0.65, 0.91,
                Form("%.2f#pm%.2f (mm)", meanY, sigmaY));
        latexY->SetNDC();
        latexY->SetTextColor(kRed);
        latexY->SetTextSize(0.05);  
        latexY->Draw();
    }

    cProjY->Update();


    cProjY->Modified();
    cProjY->Update();
    cProjY->SaveAs("ProjY.png");

    delete h3Clone;
    delete projX;
    delete projY;
    delete cROI;
    delete cProjX;
    delete cProjY;
}

void hPlotCircle(const char* imagePath, const char* outputPath, double elevation, double azimuthal, double ppRad, double ppXRad, double ppYRad, double centerX, double centerY, double yRad, double xRad, double threshold, double transparency, int nbin, bool drawLatex, bool adjPos, bool trans, double thrs) {
    TImage* img = TImage::Open(imagePath);
    if (!img) {
        std::cerr << "Error loading or invalid image: " << imagePath << std::endl;
        delete img;
        return;
    }

    int width = img->GetWidth();
    int height = img->GetHeight();

    struct stat fileStat;
    if (stat(imagePath, &fileStat) != 0) {
        std::cerr << "Error getting file stats." << std::endl;
        return;
    }  
    std::time_t creationTime=fileStat.st_ctime;
    char timeString[100];
    std::strftime(timeString,sizeof(timeString), "%Y-%m-%d %H:%M:%S", std::localtime(&creationTime));
    cout<<timeString<<endl;

    //std::cout << "Image dimensions: " << width << " x " << height << std::endl;

    TH2F *h1 = new TH2F("h1", "Depth Map", width, -centerX, width-centerX, height, -centerY, height-centerY);

    const unsigned char* pixels = (const unsigned char*) img->GetArgbArray();
    if (!pixels) {
        std::cerr << "Error getting pixel array" << std::endl;
        delete img;
        return;
    }

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int pixelIndex = (y * width + x) * 4;  
            int grayValue = (unsigned char)pixels[pixelIndex + 1]; 
            h1->SetBinContent(x + 1, height - y, grayValue); 
        }
    }

    TCanvas *c1 = new TCanvas("c1", "1 Depth Map", 0, 0, 4096/3, 3000/3);
    gStyle->SetOptStat(0);
    h1->Draw("COLZ");
    //h1->SetMinimum(5E1);
    //h1->GetXaxis()->SetRangeUser(-xRad,xRad);
    //h1->GetYaxis()->SetRangeUser(-(yRad+5),yRad+5);
    h1->GetXaxis()->SetRangeUser(-1500,1500);
    h1->GetYaxis()->SetRangeUser(-1200,1200);
    //c1->SaveAs("Depth_Map_default.png");

    TLine *majorAxis_test = new TLine(-xRad, 0, xRad, 0);
    majorAxis_test->SetLineColor(kRed);
    majorAxis_test->SetLineWidth(5);
    majorAxis_test->Draw("same");

    TLine *minorAxis_test = new TLine(0, -(yRad+5), 0, yRad+5);
    minorAxis_test->SetLineColor(kRed);
    minorAxis_test->SetLineWidth(5);
    minorAxis_test->Draw("same");

    double radElevation = TMath::DegToRad() * elevation;
    //cout<<"radElevetion: " <<TMath::DegToRad()<<" * "<<elevation<<" = "<<radElevation<<endl;
    double transformedRadius = yRad * TMath::Cos(radElevation)/elevation;
    //cout<<"cirle Raius: " <<yRad<<" * "<<TMath::Cos(radElevation)<<" / "<<elevation<<endl;
    //cout<<"transformedRadius: " <<radElevation<<endl;
    double pixelToMmX = ppXRad / xRad;
    //cout<<"pixelToMmX: " <<yRad<<" * "<<TMath::Cos(radElevation)<<" = "<<pixelToMmX<<endl;
    double pixelToMmY = ppRad / yRad;
    //cout<<"pixelToMmY: " <<pixelToMmY<<endl;

    int newWidth = static_cast<int>(width * pixelToMmX);
    int newHeight = static_cast<int>(height * pixelToMmY);
    //cout<<newWidth<<" x "<<newHeight<<endl;

    TH2F *h2 = new TH2F("h2", "Scaled Depth Map", newWidth*nbin, -newWidth/2.0, newWidth/2.0, newHeight*nbin, -newHeight/2.0, newHeight/2.0);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int pixelIndex = (y * width + x) * 4;
            int grayValue = (unsigned char)pixels[pixelIndex + 1];
            double newX = (x + 1 - centerX) * pixelToMmX;
            double newY = (height - y - centerY) * pixelToMmY;
            h2->Fill(newX, newY, grayValue);
        }
    }

    double totalBinContent = 0;
    for(int xBin = 1; xBin <= h1->GetNbinsX(); ++xBin){
        for (int yBin = 1; yBin <= h1->GetNbinsY(); ++yBin){
            totalBinContent += h1->GetBinContent(xBin, yBin);
        }
    }

    if (totalBinContent < 1e4) {
        std::cerr << "Too little content in image → skip\n";
        delete img;
        delete h1;
        return;
    }

    //if(totalBinContent<=threshold){
    //        //cout<<"###########################################"<<endl;
    //        cout<<"########## NO BEAM ON THE TARGET ##########\n########## File will be deleted! ##########"<<endl;
    //        //cout<<"###########################################"<<endl;
    //        delete img;
    //        if(std::remove(imagePath)==0){
    //    	    deleteFile(imagePath);
    //        	    //cout<<"File deleted!"<<endl;
    //        }
    //        else{
    //        cerr<<"Error deleting file"<<endl;
    //        }
    //        //return;
    //}

    int nBinsX = h2->GetNbinsX();
    int nBinsY = h2->GetNbinsY();

    double totalCount = 0;
    double totalErrorSquared = 0;
    double totalCount_15 = 0;
    double totalErrorSquared_15 = 0;
    double totalCount_bkg = 0;
    double totalErrorSquared_bkg = 0;


    if (trans) {
        for (int binX = 1; binX <= nBinsX; ++binX) {
            for (int binY = 1; binY <= nBinsY; ++binY) {
                double binCenterX = h2->GetXaxis()->GetBinCenter(binX);
                double binCenterY = h2->GetYaxis()->GetBinCenter(binY);
                double dist = TMath::Sqrt(TMath::Power(binCenterX - 0.3, 2) + TMath::Power(binCenterY - 0, 2));

                //cout<<"binCenterX: "<<binCenterX<<", "<<dist<<endl;
                if (dist < 4) {
                    //h2->SetBinContent(binX, binY, 1000);
                    double binContent = h2->GetBinContent(binX, binY);
                    double binError = h2->GetBinError(binX, binY);

                    totalCount += binContent;
                    totalErrorSquared += binError * binError;

                    //std::cout << "Bin(" << binX << ", " << binY << ", Dist: "<<dist<<"): Content = " << binContent << ", Error = " << binError << std::endl;
                }
                else if (dist > 4 && dist < 6){
                    //h2->SetBinContent(binX, binY, 1000);
                    double binContent = h2->GetBinContent(binX, binY);
                    double binError = h2->GetBinError(binX, binY);

                    totalCount_15 += binContent;
                    totalErrorSquared_15 += binError * binError;

                    //std::cout << "Bin(" << binX << ", " << binY << ", Dist: "<<dist<<"): Content = " << binContent << ", Error = " << binError << std::endl;
                }
                else if (dist > 6 && dist < 20){
                    //h2->SetBinContent(binX, binY, 1000);
                    double binContent = h2->GetBinContent(binX, binY);
                    double binError = h2->GetBinError(binX, binY);

                    totalCount_bkg += binContent;
                    totalErrorSquared_bkg += binError * binError;

                    //std::cout << "Bin(" << binX << ", " << binY << ", Dist: "<<dist<<"): Content = " << binContent << ", Error = " << binError << std::endl;
                }

            }
        }

        double totalError = TMath::Sqrt(totalErrorSquared);
        double totalError_15 = TMath::Sqrt(totalErrorSquared_15);
        double totalError_bkg = TMath::Sqrt(totalErrorSquared_bkg);

        std::cout << "Total count in circle: " << totalCount <<", Err: "<<totalError<<"    in 15mm: "<<totalCount_15 <<", Err: "<<totalError_15<< std::endl;
        cout<<"ratio: "<<totalCount/(totalCount+totalCount_15)*100<<" %"<<endl;
    }

    TCanvas *c2 = new TCanvas("c2", "Transformed Depth Map", 0, 0, 3370/3, 2925/3);
    //TCanvas *c2 = new TCanvas("c2", "Transformed Depth Map", 0, 0, 3400/3, 3000/3);
    gStyle->SetOptStat(0);
    //h2->Draw("COLZ");
    gStyle->SetPalette(1);
    h2->Draw("COLZ1");
    h2->SetMinimum(thrs);
    gPad->Update();
    auto palette = (TPaletteAxis*)h2->GetListOfFunctions()->FindObject("palette");
    //palette->SetY2NDC(0.7);
    //h2->SetTitle("Beam Position Monitor (phospore)");
    //h2->SetTitle("KoBRA F0 phosphor:)");
    h2->SetTitle("");
    cout<<totalBinContent<<endl;
    gStyle->SetTitleAlign(22);
    h2->GetXaxis()->SetRangeUser(-ppXRad-1, ppXRad+1);
    h2->GetYaxis()->SetRangeUser(-ppRad-1, ppRad+1);
    h2->GetXaxis()->SetTitle("X (mm)");
    h2->GetYaxis()->SetTitle("Y (mm)");
    h2->GetXaxis()->CenterTitle(true);
    h2->GetYaxis()->CenterTitle(true);

    h2->GetYaxis()->SetTitleOffset(1.);
    h2->GetXaxis()->SetLabelSize(0.05);
    h2->GetXaxis()->SetTitleSize(0.05);
    h2->GetYaxis()->SetLabelSize(0.05);
    h2->GetYaxis()->SetTitleSize(0.05);

    //TEllipse *ellipse = new TEllipse(0, 0, transformedRadius, transformedRadius);
    TEllipse *ellipse = new TEllipse(0, 0, ppXRad, ppRad);
    ellipse->SetFillColorAlpha(kGray+1, transparency); 
    ellipse->SetLineColor(kGray+3); 
    ellipse->SetLineWidth(6); 
    ellipse->SetFillStyle(0); 
    ellipse->Draw("same");

    TEllipse *ell_target = new TEllipse(0, 0, 4, 4);
    ell_target->SetFillColorAlpha(kRed, transparency);
    ell_target->SetLineColor(kRed);
    ell_target->SetLineStyle(2);
    ell_target->SetLineWidth(3);
    ell_target->SetFillStyle(0);
    if(trans) ell_target->Draw("same");

    TEllipse *ell_bkg = new TEllipse(0, 0, 10, 10);
    ell_bkg->SetFillColorAlpha(kGray+1, transparency);
    ell_bkg->SetLineColor(kGray+3);
    ell_bkg->SetLineStyle(2);
    ell_bkg->SetLineWidth(2);
    ell_bkg->SetFillStyle(0);
    if(trans) ell_bkg->Draw("same");

    int numLines = 40;
    double xStep = 1;  // -15mm to 15mm in x-axis
    double yStep = 1;  // -15mm to 15mm in y-axis

    for (int i = 0; i <= numLines; ++i) {
        double y = -ppYRad + i * yStep;
        if (y >= -ppYRad && y <= ppYRad) {
            double x1 = -ppXRad * sqrt(1 - ((y * y) / (ppYRad * ppYRad)));
            //cout<<x1<<endl;
            double x2 = ppXRad * sqrt(1 - ((y * y) / (ppYRad * ppYRad)));
            //cout<<x2<<endl;

            TLine *lineH = new TLine(x1, y, x2, y);
            lineH->SetLineColor(kGray+3);
            lineH->SetLineWidth(2);
            //lineH->SetLineStyle(2);
            lineH->Draw("same");
        }

        double x = -ppXRad + i * xStep;
        if (x >= -ppXRad && x <= ppXRad) {
            double y1 = -ppYRad * sqrt(1 - ((x * x) / (ppXRad * ppXRad)));
            double y2 = ppYRad * sqrt(1 - ((x * x) / (ppXRad * ppXRad)));

            TLine *lineV = new TLine(x, y1, x, y2);
            lineV->SetLineColor(kGray+3);
            lineV->SetLineWidth(2);
            //lineV->SetLineStyle(2);
            lineV->Draw("same");
        }
    }

    TLine *majorAxis = new TLine(-ppXRad, 0, ppXRad, 0);
    majorAxis->SetLineColor(kRed);
    majorAxis->SetLineWidth(4);
    majorAxis->Draw("same");

    TLine *minorAxis = new TLine(0, -ppYRad, 0, ppYRad);
    minorAxis->SetLineColor(kRed);
    minorAxis->SetLineWidth(4);
    minorAxis->Draw("same");

    TLatex latex0;
    latex0.SetNDC();
    latex0.SetTextSize(0.05);
    //if(totalBinContent<=threshold) latex0.DrawLatex(0.26,0.95,"KoBRA F0 (Phosphor Off)");
    if(totalBinContent<=threshold) latex0.DrawLatex(0.40,0.95,"KoBRA F0");
    else latex0.DrawLatex(0.36,0.95,"KoBRA F0 (Vert.)");

    TLatex latex1;
    latex1.SetNDC();
    latex1.SetTextSize(0.04);
    latex1.DrawLatex(0.61,0.905,Form("%s",timeString));

    TLatex latex2;
    latex2.SetNDC();
    latex2.SetTextSize(0.04);
    latex2.DrawLatex(0.61,0.865,"Experimental Team");
    latex2.DrawLatex(0.79,0.815,"(G. Oh)");

    if(trans){
        TLatex latex3;
        latex3.SetNDC();
        latex3.SetTextSize(0.04);
        latex3.DrawLatex(0.12,0.865,"Target (#it{#phi} 8 mm)");
        if(totalBinContent<=threshold) latex3.DrawLatex(0.13,0.815,"0.0%");
        else if (totalCount/(totalCount+totalCount_15)*100>95) latex3.DrawLatex(0.13,0.815,Form("%.1f%% lol", totalCount/(totalCount+totalCount_15)*100));
        else latex3.DrawLatex(0.13,0.815,Form("%.1f%%", totalCount/(totalCount+totalCount_15)*100));
        //latex3.DrawLatex(0.11,0.815,Form(""));
        //latex3.DrawLatex(0.11,0.815,Form("%0.f#pm%.0f", totalCount_bkg, totalError_bkg));
        //latex3.DrawLatex(0.11,0.815,Form("%0.f#pm%.0f, %.1f%%", totalCount, totalError, totalCount/(totalCount+totalCount_15)*100));

    }

    c2->Modified();
    c2->Update();
    gSystem->ProcessEvents();
    c2->SaveAs(outputPath);

    //delete img;
    //delete c1;
    //delete c2;
    //delete h1;
    //// Analyze ROI
    //anaROI(h2, -ppXRad+13, ppXRad-13, -ppYRad+13, ppYRad-13, drawLatex);

    //delete h2;

    if (!adjPos) {
        delete img;
        delete c1;
        delete c2;
        delete h1;
    } 

    anaROI(h2, -ppXRad+13, ppXRad-13, -ppYRad+13, ppYRad-13, drawLatex);

    if (!adjPos) {
        delete h2;
    }

}

void BPM_KoBRA_F0(bool adjPos = 0, bool drawLatex = 1, bool trans = 1, double thrs = 1E2) {
//void BPM_KoBRA_F0(const char* inputDir = "./captures", 
//                  const char* outputDir = "./captures/analysis_results",
//                  bool adjPos = 0, bool drawLatex = 1, bool trans = 1, double thrs = 1E2) {
    // Parameters
    int nbin=3;
    double elevation = 45;
    double azimuthal = 0; 
    double ppRad = 20;
    double ppXRad = 20;
    double ppYRad = 20;
    double transparency = 0.1;
    double threshold = 3.08581e+06; // 1 sec
    //double threshold = 1.0e+06; // 0.5 sec

    ////////////////////////////////////////
    //////////Position Adjustment///////////
    ////////////////////////////////////////
    
    //250718 Off Vac.
    //double centerX = 2125;
    //double centerY = 1280;
    //double xRad = 552;
    //double yRad = 870;
    //250721 On Vac.
    //double centerX = 1970;
    //double centerY = 1430;
    //double xRad = 540;
    //double yRad = 884;
    //250723 Off Vac.
    //double centerX = 2105;
    //double centerY = 1420;
    //double xRad = 530;
    //double yRad = 884;
    //250723 On Vac.
    //double centerX = 1977;
    //double centerY = 1420;
    //double xRad = 530;
    //double yRad = 884;
    //250807 On Vac.
    //double centerX = 1970;
    //double centerY = 1420;
    //double xRad = 550;
    //double yRad = 895;
    //250811 ladder postion -58.9 mm -> -57.9 mm
    //double centerX = 1970;
    //double centerY = 1420;
    //double xRad = 550;
    //double yRad = 895;
    //250818 ladder postion -58.9 mm -> -57.9 mm
    //double centerX = 1740;
    //double centerY = 1420;
    //double xRad = 550;
    //double yRad = 895;
    //250819 ladder postion adjusted 
    //double centerX = 1735;
    //double centerY = 1395;
    //double xRad = 550;
    //double yRad = 895;
    //250821 ladder postion adjusted 
    //double centerX = 1740;
    //double centerY = 1395;
    //double xRad = 540;
    //double yRad = 895;
    //250825 target changed, adjusted 
    //double centerX = 1740;
    //double centerY = 1390;
    //double xRad = 545;
    //double yRad = 890;
    //250826 target changed, adjusted 
    //double centerX = 1720-145;
    //double centerY = 1395;
    //double xRad = 545;
    //double yRad = 890;
    //250909 target changed, adjusted 
    //double centerX = 2430;
    //double centerY = 1430;
    //double xRad = 545;
    //double yRad = 880;
    //250912 target changed, adjusted 
    //double centerX = 2410;
    //double centerY = 1430;
    //double xRad = 545;
    //double yRad = 880;
    //251020 target changed, adjusted 
    double centerX = 2200;
    double centerY = 1380;
    double xRad = 430;
    double yRad = 730;



    //std::string dirPath = "/mnt/c/SynologyDrive/Monitoring/MV-CH120-10GC (DA2098341)";//NDPS
    //std::string dirPath = "/mnt/c/Users/kobra/MVS/Data/MV-CH120-10GC (02F89270002)";//F0
    std::string dirPath = "/share/F0/data/MV-CH120-10GC (02F89270002)";//F0
    //std::string dirPath = "/share/F0/data/temp";//F0
    //std::string dirPath = "/mnt/c/SynologyDrive/BeamTest2024/Monitoring/test";

    std::string fPath = getFile(dirPath);

    if (fPath.empty()) {
        std::cerr << "No valid file found in directory: " << dirPath << std::endl;
        //return 1;
    }

    hPlotCircle(fPath.c_str(), "BPM.png", elevation, azimuthal, ppRad, ppXRad, ppYRad, centerX, centerY, yRad, xRad, threshold, transparency, nbin, drawLatex, adjPos, trans, thrs);
    //hPlotCircle(fPath.c_str(), "BPM.png", elevation, azimuthal, ppRad, centerX, centerY, yRad, xRad, threshold, transparency);
    //return 0;
}

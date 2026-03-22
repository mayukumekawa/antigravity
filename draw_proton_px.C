#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <TKey.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TCut.h>

void draw_proton_px(const char* filename = "phase.root") {
    // ROOTファイルを開く
    TFile *file = TFile::Open(filename);
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return;
    }

    // TTreeを取得する処理
    TTree *tree = nullptr;
    TIter next(file->GetListOfKeys());
    TKey *key;
    while ((key = (TKey*)next())) {
        if (TString(key->GetClassName()) == "TTree" || TString(key->GetClassName()) == "TNtuple") {
            tree = (TTree*)key->ReadObj();
            std::cout << "Found TTree: " << tree->GetName() << " with " << tree->GetEntries() << " entries." << std::endl;
            break;
        }
    }

    if (!tree) {
        std::cerr << "Error: No TTree found in file " << filename << std::endl;
        return;
    }

    // キャンバスを作成してDrawする
    TCanvas *can = new TCanvas("can", "Proton px Distribution", 800, 600);
    can->SetGrid();
    
    // pid == 2212 (陽子のPDGコード) を要求するカット
    TCut proton_cut = "pid == 2212";
    
    // pxの分布を陽子のカット条件付きで描画
    std::cout << "Drawing px distribution for protons (pid == 2212)..." << std::endl;
    tree->Draw("px", proton_cut);
    
    // オプション: 描画されたヒストグラムの見た目を調整する
    TH1F *htemp = (TH1F*)gPad->GetPrimitive("htemp");
    if (htemp) {
        htemp->SetTitle("Proton p_{x} Distribution;p_{x} (GeV/c);Counts");
        htemp->SetLineColor(kRed + 1);
        htemp->SetFillColor(kPink + 7); // 色を赤系に
        htemp->SetLineWidth(2);
    }
    
    can->Update();

    // 画像ファイルとして保存する
    can->SaveAs("proton_px_distribution.pdf");
    std::cout << "Saved histogram to proton_px_distribution.pdf" << std::endl;
}

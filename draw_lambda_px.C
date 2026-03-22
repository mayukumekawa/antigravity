#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <TKey.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TCut.h>

void draw_lambda_px(const char* filename = "phase.root") {
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
    TCanvas *can = new TCanvas("can", "Lambda px Distribution", 800, 600);
    can->SetGrid();
    
    // pid == 3122 (Λ粒子のPDGコード) を要求するカット
    TCut lambda_cut = "pid == 3122";
    
    // pxの分布をΛ粒子のカット条件付きで描画
    std::cout << "Drawing px distribution for Lambda hyperons (pid == 3122)..." << std::endl;
    tree->Draw("px", lambda_cut);
    
    // オプション: 描画されたヒストグラムの見た目を調整する
    TH1F *htemp = (TH1F*)gPad->GetPrimitive("htemp");
    if (htemp) {
        htemp->SetTitle("#Lambda p_{x} Distribution;p_{x} (GeV/c);Counts");
        htemp->SetLineColor(kBlue + 1);
        htemp->SetFillColor(kCyan - 9); // 色を青系に
        htemp->SetLineWidth(2);
    }
    
    can->Update();

    // 画像ファイルとして保存する
    can->SaveAs("lambda_px_distribution.pdf");
    std::cout << "Saved histogram to lambda_px_distribution.pdf" << std::endl;
}

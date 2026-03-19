#include <iostream>
#include <TFile.h>
#include <TTree.h>
#include <TKey.h>
#include <TCanvas.h>
#include <TH1F.h>

void draw_Npart(const char* filename = "phase.root") {
    // ROOTファイルを開く
    TFile *file = TFile::Open(filename);
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return;
    }

    // TTreeを取得する処理（Tree名が不明でもファイル内の最初のTreeを見つける）
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
        std::cerr << "Contents of the file:" << std::endl;
        file->ls();
        return;
    }

    // キャンバスを作成してDrawする
    TCanvas *can = new TCanvas("can", "Npart Distribution", 800, 600);
    can->SetGrid();
    
    // Npartを描画
    tree->Draw("Npart");

    
    // オプション: 描画されたヒストグラムの見た目を調整する
    TH1F *htemp = (TH1F*)gPad->GetPrimitive("htemp");
    if (htemp) {
        htemp->SetTitle("Npart Distribution;Npart;Counts");
        htemp->SetLineColor(kBlue + 1);
        htemp->SetFillColor(kAzure + 7);
        htemp->SetLineWidth(2);
    }
    
    can->Update();

    // X転送ができない環境向けに、画像ファイルとして保存する
    can->SaveAs("Npart_distribution.pdf");
    std::cout << "Saved histogram to Npart_distribution.pdf" << std::endl;
}

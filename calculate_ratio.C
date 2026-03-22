#include <iostream>
#include <vector>
#include <deque>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TVector3.h"

void calculate_ratio(const char* filename = "phase.root") {
    // Open the ROOT file
    TFile *file = TFile::Open(filename);
    if (!file || file->IsZombie()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return;
    }

    // Get the TTree
    TTree *tree = (TTree*)file->Get("tree");
    if (!tree) {
        std::cerr << "Error: Cannot find TTree 'tree'" << std::endl;
        return;
    }

    // Pointers for branch reading
    std::vector<int> *pid = nullptr;
    std::vector<float> *px = nullptr;
    std::vector<float> *py = nullptr;
    std::vector<float> *pz = nullptr;

    tree->SetBranchAddress("pid", &pid);
    tree->SetBranchAddress("px", &px);
    tree->SetBranchAddress("py", &py);
    tree->SetBranchAddress("pz", &pz);

    // True (Same Event) Histogram
    TH1F *h_true = new TH1F("h_true", "Same Event Relative Momentum", 100, 0, 5);
    h_true->Sumw2(); // 割り算時に誤差を正しく計算させるため

    // Mixed Event Histogram
    TH1F *h_mixed = new TH1F("h_mixed", "Mixed Event Relative Momentum", 100, 0, 5);
    h_mixed->Sumw2();

    int nEvents = tree->GetEntries();
    std::cout << "Starting processing " << nEvents << " events..." << std::endl;

    std::deque<std::vector<TVector3>> lambda_buffer;
    const size_t kNmix = 5;

    for (int i = 0; i < nEvents; ++i) {
        tree->GetEntry(i);

        std::vector<TVector3> protons;
        std::vector<TVector3> lambdas;

        // Collect protons and lambdas in current event
        for (size_t j = 0; j < pid->size(); ++j) {
            if ((*pid)[j] == 2212) { // Proton
                protons.emplace_back((*px)[j], (*py)[j], (*pz)[j]);
            } else if ((*pid)[j] == 3122) { // Lambda
                lambdas.emplace_back((*px)[j], (*py)[j], (*pz)[j]);
            }
        }

        // True pairs (Same event)
        for (const auto& p_prot : protons) {
            for (const auto& p_lam : lambdas) {
                TVector3 p_rel = p_prot - p_lam;
                h_true->Fill(p_rel.Mag());
            }
        }

        // Mixed pairs (Mixed event)
        for (const auto& p_prot : protons) {
            for (const auto& prev_lambdas : lambda_buffer) {
                for (const auto& p_lam : prev_lambdas) {
                    TVector3 p_rel = p_prot - p_lam;
                    h_mixed->Fill(p_rel.Mag());
                }
            }
        }

        // Update buffer
        lambda_buffer.push_back(lambdas);
        if (lambda_buffer.size() > kNmix) {
            lambda_buffer.pop_front();
        }

        if (i % 10000 == 0 && i > 0) {
            std::cout << "Processed " << i << " / " << nEvents << " events" << std::endl;
        }
    }

    // Ratio (Correlation Function)
    TH1F *h_ratio = (TH1F*)h_true->Clone("h_ratio");
    h_ratio->SetTitle("Proton-#Lambda Correlation Function;k* = |#vec{p}_{p} - #vec{p}_{#Lambda}| (GeV/c);C(k*)");

    Double_t nTrue = h_true->Integral();
    Double_t nMixed = h_mixed->Integral();

    if (nMixed > 0 && nTrue > 0) {
        // 同じイベントとMixed Eventのペア数の違いを補正（比が1に漸近するようにスケール）
        h_ratio->Divide(h_mixed);
        h_ratio->Scale(nMixed / nTrue);
    }

    TCanvas *c1 = new TCanvas("c1", "Correlation Function", 800, 600);
    h_ratio->Draw("E"); // 誤差棒付きで描画
    
    // y=1の基準線を追加
    TLine *line = new TLine(0, 1, 5, 1);
    line->SetLineStyle(2); // 破線
    line->Draw();

    c1->SaveAs("correlation_function.pdf");

    std::cout << "Macro finished. Histogram saved as correlation_function.pdf" << std::endl;
    file->Close();
}

#include <iostream>
#include <vector>
#include <deque>
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TVector3.h"

void calculate_mixed_relative_momentum(const char* filename = "phase.root") {
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

    // Define histogram for relative momentum |p_p - p_L|
    TH1F *h_rel_mom_mixed = new TH1F("h_rel_mom_mixed", "Relative Momentum |#vec{p}_{p} - #vec{p}_{#Lambda}| (Mixed Event);|#vec{p}_{p} - #vec{p}_{#Lambda}| (GeV/c);Counts", 100, 0, 5);

    int nEvents = tree->GetEntries();
    std::cout << "Starting processing " << nEvents << " events for Mixed Event Analysis..." << std::endl;

    // Buffer to store lambdas from previous events for the mixed event analysis
    std::deque<std::vector<TVector3>> lambda_buffer;
    const size_t kNmix = 5; // Number of distinct previous events to mix with

    for (int i = 0; i < nEvents; ++i) {
        tree->GetEntry(i);

        std::vector<TVector3> protons;
        std::vector<TVector3> lambdas;

        // Collect all protons and lambdas in the current event
        for (size_t j = 0; j < pid->size(); ++j) {
            if ((*pid)[j] == 2212) { // Proton
                protons.emplace_back((*px)[j], (*py)[j], (*pz)[j]);
            } else if ((*pid)[j] == 3122) { // Lambda
                lambdas.emplace_back((*px)[j], (*py)[j], (*pz)[j]);
            }
        }

        // Mixed event pairing: pair current protons with lambdas from previous events
        for (const auto& p_prot : protons) {
            for (const auto& prev_lambdas : lambda_buffer) {
                for (const auto& p_lam : prev_lambdas) {
                    TVector3 p_rel = p_prot - p_lam; // 運動量の引き算
                    h_rel_mom_mixed->Fill(p_rel.Mag()); // 絶対値をとってヒストグラムに詰める
                }
            }
        }

        // Update the lambda event buffer
        lambda_buffer.push_back(lambdas);
        if (lambda_buffer.size() > kNmix) {
            lambda_buffer.pop_front(); // 最も古いイベントのΛを削除してバッファサイズを一定に保つ
        }

        if (i % 10000 == 0 && i > 0) {
            std::cout << "Processed " << i << " / " << nEvents << " events" << std::endl;
        }
    }

    // Draw and save the histogram
    TCanvas *c1 = new TCanvas("c1", "Mixed Event Relative Momentum", 800, 600);
    h_rel_mom_mixed->Draw();
    c1->SaveAs("mixed_relative_momentum_histogram.pdf");

    std::cout << "Macro finished. Histogram saved as mixed_relative_momentum_histogram.pdf" << std::endl;

    file->Close();
}

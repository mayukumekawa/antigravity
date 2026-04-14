#include <TFile.h>
#include <TTree.h>
#include <TVector3.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <vector>

using namespace std;

void npart165() {

    TFile *file = TFile::Open("phase.root");
    TTree *tree = (TTree*)file->Get("tree");

    Int_t Npart;
    vector<int> *pid = 0;
    vector<float> *px = 0;
    vector<float> *py = 0;
    vector<float> *pz = 0;

    tree->SetBranchAddress("Npart", &Npart);
    tree->SetBranchAddress("pid", &pid);
    tree->SetBranchAddress("px", &px);
    tree->SetBranchAddress("py", &py);
    tree->SetBranchAddress("pz", &pz);

    TH1D *h_q = new TH1D("h_q", "Relative Momentum;p (GeV/c);Counts", 100, 0, 2);

    Long64_t nentries = tree->GetEntries();

    for (Long64_t i = 0; i < nentries; i++) {
        tree->GetEntry(i);

        if (Npart < 165.0 || Npart > 187.7) continue;

        for (size_t j = 0; j < pid->size(); j++) {
            if ((*pid)[j] != 2212) continue;

            TVector3 p_proton((*px)[j], (*py)[j], (*pz)[j]);

            for (size_t k = 0; k < pid->size(); k++) {
                if ((*pid)[k] != 3122) continue;

                TVector3 p_lambda((*px)[k], (*py)[k], (*pz)[k]);

                double q = (p_proton - p_lambda).Mag();
                h_q->Fill(q);
            }
        }
    }

    TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
    h_q->Draw();
    c1->SaveAs("npart165.pdf");
}

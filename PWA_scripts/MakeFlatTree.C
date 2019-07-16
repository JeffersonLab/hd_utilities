//root -l MakeFlatTree.C

void MakeFlatTree()
{
    //Get flat tree from DSelector
    TFile *fileIn=new TFile("flat_tree_pippimeta_orien0.root");
    TTree *tree=(TTree*)fileIn->Get("pippimeta__B4_M17");

    //Define 4 vectors
    TLorentzVector* beam_p4_kin = 0;
    TLorentzVector* pip_p4_kin = 0;
    TLorentzVector* pim_p4_kin = 0;
    TLorentzVector* p_p4_kin = 0;
    TLorentzVector* g1_p4_kin = 0;
    TLorentzVector* g2_p4_kin = 0;

    //get 4 vectors
    tree->SetBranchAddress("beam_p4_kin", &beam_p4_kin );
    tree->SetBranchAddress("pip_p4_kin", &pip_p4_kin );
    tree->SetBranchAddress("pim_p4_kin", &pim_p4_kin );
    tree->SetBranchAddress("p_p4_kin", &p_p4_kin );
    tree->SetBranchAddress("g1_p4_kin", &g1_p4_kin );
    tree->SetBranchAddress("g2_p4_kin", &g2_p4_kin );

    //Get other varriables you may have defined in flat tree
    //double KinFitChiSqPerNDF;
    //tree->SetBranchAddress("KinFitChiSqPerNDF",&KinFitChiSqPerNDF);

    int m_nPart;
    int m_PID[4];
    float m_e[4];
    float m_px[4];
    float m_py[4];
    float m_pz[4];
    float m_eBeam;
    float m_pxBeam;
    float m_pyBeam;
    float m_pzBeam;
    float m_weight;
    float m_TargetMass;
    
    //Create AmpTools output file
    TFile *outFile=new TFile("Accepted_Data.root","RECREATE");
    TTree *m_OutTree= new TTree("kin", "kin");

    m_OutTree->Branch("E_Beam", &m_eBeam, "E_Beam/F");
    m_OutTree->Branch("Px_Beam", &m_pxBeam, "Px_Beam/F");
    m_OutTree->Branch("Py_Beam", &m_pyBeam, "Py_Beam/F");
    m_OutTree->Branch("Pz_Beam", &m_pzBeam, "Pz_Beam/F");
    m_OutTree->Branch("Weight", &m_weight, "Weight/F");
    
    m_OutTree->Branch("E_FinalState",m_e, "E_FinalState[4]/F");
    m_OutTree->Branch("Px_FinalState", m_px, "Px_FinalState[4]/F");
    m_OutTree->Branch("Py_FinalState", m_py, "Py_FinalState[4]/F");
    m_OutTree->Branch("Pz_FinalState", m_pz, "Pz_FinalState[4]/F");
    
    m_OutTree->Branch("NumFinalState", &m_nPart,"NumFinalState/I");
    m_nPart = 4;
    
    m_OutTree->Branch("Target_Mass", &m_TargetMass, "Target_Mass/F");
    m_TargetMass = 0.938272;          // Proton mass in GeV.
    
    m_OutTree->Branch("PID_FinalState", m_PID, "PID_FinalState[4]/I");
    m_PID[0] = 14;//proton
    m_PID[1] = 17;//eta
    m_PID[2] = 9;//pi-
    m_PID[3] = 8;//pi+

    Long64_t NEntries=tree->GetEntries();
    cout<<"There are "<<NEntries<<" entries."<<endl;
    
    for (Long64_t i_entry=0; i_entry<NEntries; i_entry++) {
        tree->GetEntry(i_entry);
        if (i_entry%250000==0)                            //outputs every 500,000 events processed
            cout<<i_entry<<" events processed"<<endl;
        
        TLorentzVector EtaP4;
        EtaP4.SetPxPyPzE(g1_p4_kin->Px() + g2_p4_kin->Px() , g1_p4_kin->Py() + g2_p4_kin->Py(), g1_p4_kin->Pz() + g2_p4_kin->Pz(), g1_p4_kin->E() + g2_p4_kin->E());
        
        m_e[3] = pip_p4_kin->E();
        m_px[3] = pip_p4_kin->Px();
        m_py[3] = pip_p4_kin->Py();
        m_pz[3] = pip_p4_kin->Pz();
        m_e[2] = pim_p4_kin->E();
        m_px[2] = pim_p4_kin->Px();
        m_py[2] = pim_p4_kin->Py();
        m_pz[2] = pim_p4_kin->Pz();
        m_e[1] = EtaP4.E();
        m_px[1] = EtaP4.Px();
        m_py[1] = EtaP4.Py();
        m_pz[1] = EtaP4.Pz();
        m_e[0] = p_p4_kin->E();
        m_px[0] = p_p4_kin->Px();
        m_py[0] = p_p4_kin->Py();
        m_pz[0] = p_p4_kin->Pz();
        m_eBeam = beam_p4_kin->E();
        m_pxBeam = beam_p4_kin->Px();
        m_pyBeam = beam_p4_kin->Py();
        m_pzBeam = beam_p4_kin->Pz();
        m_weight= 1; //Can fill with sidebandWeight*tagWeight (or combo weight)
        
        m_OutTree->Fill();
    }

    m_OutTree->Write();
    outFile->Close();

}

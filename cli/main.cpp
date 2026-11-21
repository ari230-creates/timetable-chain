#include "core/dataset.hpp"
#include "core/generator.hpp"
#include "core/constraint_checker.hpp"
#include "core/blockchain.hpp"

#include <iostream>
#include <iomanip>
#include <filesystem>

int main(int argc, char* argv[]) {
    std::cout << "========================================================\n";
    std::cout << "  Timetable Chain - C++ Project CLI Demo & Verifier     \n";
    std::cout << "========================================================\n\n";

    std::string dataPath = "data/sample.txt";
    if (argc > 1) {
        dataPath = argv[1];
    } else if (!std::filesystem::exists(dataPath)) {
        if (std::filesystem::exists("../data/sample.txt")) {
            dataPath = "../data/sample.txt";
        }
    }

    core::Dataset ds;
    std::string loadErr;
    if (!ds.loadFromFile(dataPath, loadErr)) {
        std::cerr << "Error loading dataset: " << loadErr << "\n";
        return 1;
    }

    std::cout << "Loaded dataset successfully from: " << dataPath << "\n";
    std::cout << "  - Subjects: " << ds.getSubjects().size() << "\n";
    std::cout << "  - Faculty:  " << ds.getFaculty().size() << "\n";
    std::cout << "  - Slots:    " << ds.getSlots().size() << "\n";
    std::cout << "  - Total periods needed: " << ds.totalPeriodsNeeded() << "\n\n";

    core::Blockchain chain;
    std::cout << "Initialized Blockchain with Genesis block:\n";
    std::cout << "  Index: " << chain.getBlocks()[0].index
              << " | Nonce: " << chain.getBlocks()[0].nonce
              << " | Hash: " << chain.getBlocks()[0].hash << "\n\n";

    std::cout << "Running Algorithm Comparison (Plain vs MRV vs Best):\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << std::left << std::setw(10) << "Mode"
              << std::setw(14) << "Placements"
              << std::setw(14) << "Backtracks"
              << std::setw(10) << "Score"
              << std::setw(10) << "Valid?" << "\n";
    std::cout << "--------------------------------------------------------\n";

    core::Generator gen(ds);
    core::Timetable ttPlain, ttMrv, ttBest;

    gen.run(core::GeneratorMode::Plain, ttPlain);
    std::vector<std::string> p1;
    bool v1 = core::ConstraintChecker::isValid(ttPlain, ds, p1);
    std::cout << std::left << std::setw(10) << "Plain"
              << std::setw(14) << gen.getStats().placements
              << std::setw(14) << gen.getStats().backtracks
              << std::setw(10) << gen.getStats().score
              << std::setw(10) << (v1 ? "YES" : "NO") << "\n";

    gen.run(core::GeneratorMode::MRV, ttMrv);
    std::vector<std::string> p2;
    bool v2 = core::ConstraintChecker::isValid(ttMrv, ds, p2);
    std::cout << std::left << std::setw(10) << "MRV"
              << std::setw(14) << gen.getStats().placements
              << std::setw(14) << gen.getStats().backtracks
              << std::setw(10) << gen.getStats().score
              << std::setw(10) << (v2 ? "YES" : "NO") << "\n";

    gen.run(core::GeneratorMode::Best, ttBest);
    std::vector<std::string> p3;
    bool v3 = core::ConstraintChecker::isValid(ttBest, ds, p3);
    std::cout << std::left << std::setw(10) << "Best"
              << std::setw(14) << gen.getStats().placements
              << std::setw(14) << gen.getStats().backtracks
              << std::setw(10) << gen.getStats().score
              << std::setw(10) << (v3 ? "YES" : "NO") << "\n";
    std::cout << "--------------------------------------------------------\n\n";

    std::cout << "Mining timetable solutions into blockchain ledger...\n";
    chain.addBlock("MRV Generated Schedule", ttMrv.serialize(), ds);
    chain.addBlock("Optimized Best Schedule", ttBest.serialize(), ds);

    std::cout << "Blockchain ledger now contains " << chain.size() << " blocks.\n";
    for (const auto& b : chain.getBlocks()) {
        std::cout << "  Block #" << b.index << " [" << b.note << "] Nonce: " << b.nonce
                  << " Hash: " << b.hash.substr(0, 16) << "...\n";
    }

    std::cout << "\nVerifying Blockchain Ledger (3-layer verification)...\n";
    core::VerificationReport report = chain.verify(ds);
    if (report.allValid) {
        std::cout << "  ==> RESULT: Chain intact and every timetable valid! [PASS]\n";
    } else {
        std::cout << "  ==> RESULT: Verification failed!\n";
        for (const auto& m : report.messages) {
            std::cout << "      " << m << "\n";
        }
    }

    std::cout << "\n========================================================\n";
    std::cout << "CLI Demo Finished Successfully.\n";
    return 0;
}

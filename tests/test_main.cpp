#include "core/sha256.hpp"
#include "core/subject.hpp"
#include "core/faculty.hpp"
#include "core/timeslot.hpp"
#include "core/entry.hpp"
#include "core/timetable.hpp"
#include "core/dataset.hpp"
#include "core/constraint_checker.hpp"
#include "core/scorer.hpp"
#include "core/generator.hpp"
#include "core/block.hpp"
#include "core/blockchain.hpp"

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <filesystem>

void test_sha256() {
    std::cout << "[RUNNING] test_sha256...\n";
    // Standard test vectors
    assert(core::sha256("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    assert(core::sha256("") == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    assert(core::sha256(std::string(1000, 'a')) == "41edece42d63e8d9bf515a9ba6932e1c20cbc9f5a5d134645adb5db1b9737ea3");
    std::cout << "[PASSED] test_sha256\n";
}

void test_constraint_checker() {
    std::cout << "[RUNNING] test_constraint_checker...\n";
    core::Dataset ds;
    ds.addSubject(core::Subject(1, "Data Structures", 2));
    ds.addSubject(core::Subject(2, "Algorithms", 1));

    core::Faculty f1(1, "Dr. Rao");
    f1.canTeachSubjects.insert(1); // can only teach subject 1
    f1.unavailableSlots.insert(2); // unavailable in slot 2
    ds.addFaculty(f1);

    core::Faculty f2(2, "Prof. Devi");
    f2.canTeachSubjects.insert(2);
    ds.addFaculty(f2);

    ds.addSlot(core::TimeSlot(1, "Mon", 1));
    ds.addSlot(core::TimeSlot(2, "Mon", 2));
    ds.addSlot(core::TimeSlot(3, "Mon", 3));

    core::Timetable tt;
    std::string reason;

    // Rule 1: faculty cannot teach the subject
    bool ok1 = core::ConstraintChecker::canPlace(tt, *ds.findSubject(2), *ds.findFaculty(1), *ds.findSlot(1), reason);
    assert(!ok1);
    assert(reason.find("cannot teach") != std::string::npos);

    // Rule 2: faculty is unavailable in that slot
    bool ok2 = core::ConstraintChecker::canPlace(tt, *ds.findSubject(1), *ds.findFaculty(1), *ds.findSlot(2), reason);
    assert(!ok2);
    assert(reason.find("unavailable") != std::string::npos);

    // Place an entry
    tt.addEntry({1, 1, 1}); // Subj 1, Fac 1, Slot 1

    // Rule 3: slot already used (single class group)
    bool ok3 = core::ConstraintChecker::canPlace(tt, *ds.findSubject(2), *ds.findFaculty(2), *ds.findSlot(1), reason);
    assert(!ok3);
    assert(reason.find("already occupied") != std::string::npos);

    // Rule 4: faculty already teaching in that slot
    // If we test an entry for the same faculty in slot 1 (even if hypothetically checking another class)
    bool ok4 = core::ConstraintChecker::canPlace(tt, *ds.findSubject(1), *ds.findFaculty(1), *ds.findSlot(1), reason);
    assert(!ok4); // occupied / faculty already teaching

    // Test whole timetable validity (isValid)
    std::vector<std::string> problems;
    // Rule 5: subject period count mismatch
    bool valid = core::ConstraintChecker::isValid(tt, ds, problems);
    assert(!valid);
    bool foundRule5 = false;
    for (const auto& p : problems) {
        if (p.find("periods scheduled") != std::string::npos) foundRule5 = true;
    }
    assert(foundRule5);

    std::cout << "[PASSED] test_constraint_checker\n";
}

void test_generator(const std::string& samplePath) {
    std::cout << "[RUNNING] test_generator on " << samplePath << "...\n";
    core::Dataset ds;
    std::string err;
    bool loaded = ds.loadFromFile(samplePath, err);
    if (!loaded) {
        std::cerr << "Failed to load sample data from " << samplePath << ": " << err << "\n";
        assert(false);
    }

    core::Timetable ttPlain;
    core::Generator genPlain(ds);
    bool okPlain = genPlain.run(core::GeneratorMode::Plain, ttPlain);
    assert(okPlain);
    assert(genPlain.getStats().placements == 320);
    assert(genPlain.getStats().backtracks == 312);
    assert(genPlain.getStats().score == 9);
    std::vector<std::string> probPlain;
    assert(core::ConstraintChecker::isValid(ttPlain, ds, probPlain));

    core::Timetable ttMrv;
    core::Generator genMrv(ds);
    bool okMrv = genMrv.run(core::GeneratorMode::MRV, ttMrv);
    assert(okMrv);
    assert(genMrv.getStats().placements == 8);
    assert(genMrv.getStats().backtracks == 0);
    assert(genMrv.getStats().score == 16);
    std::vector<std::string> probMrv;
    assert(core::ConstraintChecker::isValid(ttMrv, ds, probMrv));

    core::Timetable ttBest;
    core::Generator genBest(ds);
    bool okBest = genBest.run(core::GeneratorMode::Best, ttBest);
    assert(okBest);
    assert(genBest.getStats().placements == 73);
    assert(genBest.getStats().backtracks == 65);
    assert(genBest.getStats().score == 7);
    std::vector<std::string> probBest;
    assert(core::ConstraintChecker::isValid(ttBest, ds, probBest));

    // Impossible data test: subject requires more periods than teacher has available slots
    core::Dataset impossibleDs;
    impossibleDs.addSubject(core::Subject(1, "Impossible Course", 5));
    core::Faculty fImp(1, "Solo Teacher");
    fImp.canTeachSubjects.insert(1);
    fImp.unavailableSlots.insert(2); // only slots 1, 3 available (2 slots < 5 needed)
    impossibleDs.addFaculty(fImp);
    impossibleDs.addSlot(core::TimeSlot(1, "Mon", 1));
    impossibleDs.addSlot(core::TimeSlot(2, "Mon", 2));
    impossibleDs.addSlot(core::TimeSlot(3, "Mon", 3));

    core::Timetable ttImp;
    core::Generator genImp(impossibleDs);
    bool okImp = genImp.run(core::GeneratorMode::Plain, ttImp);
    assert(!okImp);

    std::cout << "[PASSED] test_generator\n";
}

void test_blockchain(const std::string& samplePath) {
    std::cout << "[RUNNING] test_blockchain...\n";
    core::Dataset ds;
    std::string err;
    assert(ds.loadFromFile(samplePath, err));

    core::Timetable tt1, tt2;
    core::Generator gen(ds);
    assert(gen.run(core::GeneratorMode::Plain, tt1));
    assert(gen.run(core::GeneratorMode::Best, tt2));

    core::Blockchain chain;
    assert(chain.size() == 1); // Genesis block

    // Add Block 1
    std::string errAdd;
    bool added1 = chain.addBlock("Plain Schedule", tt1.serialize(), ds, &errAdd);
    assert(added1);
    assert(chain.size() == 2);

    // Add Block 2
    bool added2 = chain.addBlock("Optimized Best Schedule", tt2.serialize(), ds, &errAdd);
    assert(added2);
    assert(chain.size() == 3);

    // Untouched chain verifies completely
    core::VerificationReport report = chain.verify(ds);
    assert(report.allValid);
    assert(report.messages.empty());

    // Tamper test 1: Careless attacker (editing data without re-mining)
    // Create copy for tampering
    core::Blockchain carelessChain = chain;
    // Alter block 1 data
    std::string tamperedData = "99,99,99;";
    carelessChain.tamperBlock(1, tamperedData, false /* do not remine */);
    core::VerificationReport carelessReport = carelessChain.verify(ds);
    assert(!carelessReport.allValid);
    bool foundHashMismatch = false;
    for (const auto& msg : carelessReport.messages) {
        if (msg.find("Block 1:") != std::string::npos &&
            msg.find("Hash mismatch") != std::string::npos) {
            foundHashMismatch = true;
        }
    }
    assert(foundHashMismatch);

    // Tamper test 2: Clever attacker (editing data AND re-mining Block 1)
    core::Blockchain cleverChain = chain;
    cleverChain.tamperBlock(1, tamperedData, true /* remine */);
    core::VerificationReport cleverReport = cleverChain.verify(ds);
    assert(!cleverReport.allValid);
    bool foundBrokenLinkAtBlock2 = false;
    for (const auto& msg : cleverReport.messages) {
        if (msg.find("Block 2:") != std::string::npos &&
            msg.find("Broken link") != std::string::npos) {
            foundBrokenLinkAtBlock2 = true;
        }
    }
    assert(foundBrokenLinkAtBlock2);

    // Test Save and Load roundtrip
    std::string tempPath = "test_chain_roundtrip.txt";
    std::string saveErr, loadErr;
    assert(chain.save(tempPath, saveErr));

    core::Blockchain loadedChain;
    assert(loadedChain.load(tempPath, loadErr));
    assert(loadedChain.size() == chain.size());

    // Verify loaded chain
    core::VerificationReport loadedReport = loadedChain.verify(ds);
    assert(loadedReport.allValid);

    std::filesystem::remove(tempPath);
    std::cout << "[PASSED] test_blockchain\n";
}

int main(int argc, char* argv[]) {
    std::cout << "========================================\n";
    std::cout << "Starting Timetable Chain Core Test Suite\n";
    std::cout << "========================================\n";

    std::string samplePath = "data/sample.txt";
    if (argc > 1) {
        samplePath = argv[1];
    } else if (!std::filesystem::exists(samplePath)) {
        if (std::filesystem::exists("../data/sample.txt")) {
            samplePath = "../data/sample.txt";
        }
    }

    test_sha256();
    test_constraint_checker();
    test_generator(samplePath);
    test_blockchain(samplePath);

    std::cout << "========================================\n";
    std::cout << "All Unit Tests Passed Successfully! [OK]\n";
    std::cout << "========================================\n";
    return 0;
}

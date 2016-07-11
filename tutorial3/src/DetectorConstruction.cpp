
#include "DetectorConstruction.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4Transform3D.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

// New includes
#include "G4SDManager.hh"
#include "G4MultiFunctionalDetector.hh"
#include "G4VPrimitiveScorer.hh"
#include "G4PSEnergyDeposit.hh"
#include "G4PSDoseDeposit.hh"


DetectorConstruction::DetectorConstruction()
  : G4VUserDetectorConstruction()
{
  DefineMaterials();
}

DetectorConstruction::~DetectorConstruction()
{} 

void DetectorConstruction::DefineMaterials()
{
  G4NistManager* man = G4NistManager::Instance();
  G4bool isotopes = false;
  /* Define simple material */
  G4double density = 1.390 * g/cm3;
  G4double a = 39.95 * g/mole;
  G4Material* lAr = new G4Material("lArgon", // Name
                                    18.,          // Z value
                                    a,            // atomic mass
                                    density);     // That thing
  
  /* Define a simple molecule */
  G4Element* H = man -> FindOrBuildElement("H", isotopes); 
  G4Element* O = man -> FindOrBuildElement("O", isotopes);
  
  G4Material* H2O = new G4Material("Water",         // name
                                    1.000 * g/cm3,  // density
                                    2);             // number of components
  H2O -> AddElement(H, 2);  // Name and number of atoms in molecule
  H2O -> AddElement(O, 1);
  
  /* Define mixture by fractional mass */
  G4Element* N = man -> FindOrBuildElement("N", isotopes);
  density = 1.290 * mg/cm3;
  G4Material* Air = new G4Material("Air", density, 2);
  Air -> AddElement(N, 70*perCent);
  Air -> AddElement(O, 30*perCent);
}


// Create our world and box
G4VPhysicalVolume* DetectorConstruction::Construct()
{
  
  G4NistManager* man = G4NistManager::Instance();
  G4Material* default_mat = man -> FindOrBuildMaterial("Air");
  G4Material* box_mat = man -> FindOrBuildMaterial("lArgon");
  G4Material* water = man -> FindOrBuildMaterial("Water");

  /*** FIRST create the WORLD ***/
  G4double worldSize = 1. * m;
  G4Box* solidWorld = new G4Box("World", worldSize, worldSize, worldSize);
  G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, default_mat, "World");
  G4VPhysicalVolume* physWorld = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0);

  G4double Box_x = 0.5 * worldSize; 
  G4double Box_y = 0.5 * worldSize;
  G4double Box_z = 0.5 * worldSize;
  
  G4Box* testBox = new G4Box("testBox", // Name
                          Box_x,    // x length
                          Box_y,    // y length
                          Box_z);   // z length
  
  G4LogicalVolume* testBox_log = new G4LogicalVolume(testBox,         // Its solid (see the box we made)
                                                     box_mat,        // Its material 
                                                      "testBox");  // Its name
  
                                  // We can place this as part of the G4Logical Volume
                                 new G4PVPlacement(0,                 // Rotation
                                                     G4ThreeVector(),   // its location
                                                     testBox_log,       // the logical volume
                                                     "testBox",            // its name
                                                     logicWorld,        // its mother volume 
                                                     false,             // boolean operations
                                                     0);                // its copy number



  /* We are now going to create a thin plate and have it act as a detector. We'll place it at the edge of the world, opposite of the incoming rays. This means that
   * it will be located at the lowest value of x in our world. Remember that our particle gun was defined at 1.,0,0
   * */
  G4Box* plate = new G4Box("plate",
                              0.1 * m,
                              worldSize,
                              worldSize);

  G4LogicalVolume* plate_log = new G4LogicalVolume(plate,
                                                   water,
                                                   "plateLV");
                                  new G4PVPlacement(0,
                                                    G4ThreeVector(0.9 * worldSize,0,0),  // Remember that we have to account for the size of the box
                                                    plate_log,
                                                    "plate",
                                                    logicWorld,
                                                    false,
                                                    0);

      
  return physWorld; // Always return the world 
}

// We are now going to create the detector here
// Remember that we will have to edit our .hh file here
void DetectorConstruction::ConstructSDandField()
{
  G4SDManager::GetSDMpointer() -> SetVerboseLevel(1);
  
  G4MultiFunctionalDetector* plate = new G4MultiFunctionalDetector("plate");
  G4VPrimitiveScorer* prim = new G4PSDoseDeposit("dose");   // We want to record the dosage to the patient
  plate -> RegisterPrimitive(prim);
  SetSensitiveDetector("plateLV", plate);
}


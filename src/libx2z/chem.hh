#ifndef CHEM_HH
#define CHEM_HH

#include "math.hh"
#include "error.hh"
#include "atom.hh"
#include "array.hh"

#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <set>

/*********************** Atomic coordinates accuracies *******************/

extern double angle_tolerance, distance_tolerance;

bool are_angles_equal (double, double);

bool are_distances_equal (double, double);

double max_bond_length(const AtomBase&, const AtomBase&);

// Molecular Geometry
//
class MolecGeom : public std::vector<Atom> {

public:
  //
  MolecGeom () {}
  
  explicit MolecGeom (int n) : std::vector<Atom>(n) {}
  
  void operator *= (const D3::Matrix&);
  
  void operator *= (double);
  
  void operator += (const D3::Vector&);
  
  void operator -= (const D3::Vector&);
}; 

// molecule oriented and useful molecular properties
//
class MolecOrient : private MolecGeom {
  //
public:
  //
  enum MT     {LINEAR, PLANE, NONLINEAR}; // molecule types
  
  enum mode_t { SYMNUM, TEST}; // calculation types

private:
  //
  MT _mt; // molecule type
  
  double _dm [3]; // distance matrix of first three atoms

public:
  //
  MolecOrient (const MolecGeom&);
  
  operator MolecGeom () const { return *this; }

  int  sym_num       () const ;
  bool is_enantiomer () const ;
  bool is_plane  () const { return _mt == PLANE; }
  bool is_linear () const { return _mt == LINEAR; }

  int size () const { return MolecGeom::size(); }
  const Atom& operator [] (int i) const { return MolecGeom::operator[](i); }

  friend int compare (const MolecOrient&, const MolecOrient&, MolecOrient::mode_t);
};

int compare (const MolecOrient&, const MolecOrient&, MolecOrient::mode_t);

// connection graph
//
class PrimStruct : public ConMat<unsigned>, private MolecGeom {
  //
  std::vector<bool> _la; // linear attribute
  
public:
  //
  PrimStruct (const MolecGeom&, const std::set<std::set<int> >&);
  
  const Atom& operator [] (int i) const { return MolecGeom::operator[](i); }

  int valence (int i) const { return (*this)[i].valence(); }
  
  const char* atom_name (int i) const { return (*this)[i].name(); }
  
  int size () const { return MolecGeom::size(); }

  bool is_connected (int at0, int at1)           const { if(at0 == at1) return true; return (*this)(at0, at1); }

  bool is_connected (int, const std::list<int>&) const;

  std::list<std::list<int> > connected_group () const;

  bool is_connected () const { if(connected_group().size() == 1) return true; return false; }
  
  //int distance (int, int) const ;
  
  bool is_ring (int, int) const ;
  
  bool is_linear (int at) const { return _la[at]; }

  std::string group_stoicheometry (const std::list<int>& group) const;
};

// bond attributes
//
enum {
  GEN_BOND = 0, // generic bond
  LIN_BOND = 1, // linear bond
  ROT_BOND = 2, // rotational bond
  BET_BOND = 4  // beta-scission bond
};


// connectivity record
//
struct ConRec {
  int atom;  // atom index
  int cref;  // connected atom reference up the tree
  int begin; // first reference in the connected group down the tree
  int end;   // first reference  out of the connected group down the tree
  int attr;  // bond attributes

  ConRec (int a, int c) : atom(a), cref(c), begin(-1), end(-1), attr(GEN_BOND) {}
};

struct BetaData {
  //
  int radical;     // radical site
  int primary;     // primary site connected to radical one
  int secondary;   // secondary site connected to primary one
  bool isring;     // is the bond in the ring structure

  BetaData () : radical(-1), primary(-1), secondary(-1), isring(false) {}

  operator bool () const { if(radical < 0) return false; return true; }

  std::set<int> bond () const { std::set<int> res; res.insert(primary); res.insert(secondary); return res; }
  
};
  
// molecular structure
//
class MolecStruct : public PrimStruct
{
  std::vector<ConMat<unsigned> > _resonance;

  std::vector<ConRec> _cpath; // connectivity scheme
  
  std::string        _zmat; // zmatrix structure
  
  std::map<int, std::list<std::list<int> > > _rotvar; // rotational bonds
  
  std::map<int, BetaData> _betvar; // beta-scission bonds
  
  MultiArray<double>  _coval; // initial values of z-matrix coordinates

  std::list<int> _constvar; // constants

  std::map<int, int> _atom_map; // atom-to-zmatrix map

public:
  
  enum {
    DISTANCE  = 0,
    POLAR     = 1,
    DIHEDRAL  = 2
  };

  static const char* var_name (int);

  MolecStruct (const PrimStruct&, const std::set<std::set<int> >&) ;

  int resonance_count () const { return _resonance.size(); }

  std::vector<int> atom_ordering() const;
 
  double bond_order (int, int) const;
  
  void print (std::ostream&, const std::string& = std::string()) const;

  bool is_single  (int at0, int at1) const ;

  BetaData is_beta    (int at0, int at1) const ;

  bool is_radical (int at)           const;

  const std::string&            zmatrix () const { return _zmat; }
  
  const std::map<int, std::list<std::list<int> > >& rotation_bond () const { return _rotvar; }
  
  const std::map<int, BetaData>&     beta_bond () const { return _betvar; }
  const std::list<int>&       const_var () const { return _constvar; }

  const MultiArray<double>&          zmat_coval () const { return _coval; }

  int atom_map (int i) const;
};

#endif

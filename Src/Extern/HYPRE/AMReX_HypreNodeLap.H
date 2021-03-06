#ifndef AMREX_HYPRE_NODE_LAP_H_
#define AMREX_HYPRE_NODE_LAP_H_
#include <AMReX_Config.H>

#include <memory>
#include <algorithm>

#include <AMReX_Geometry.H>
#include <AMReX_MultiFab.H>
#include <AMReX_iMultiFab.H>
#include <AMReX_LayoutData.H>
#include <AMReX_BndryData.H>
#include <AMReX_MultiFabUtil.H>
#include <AMReX_HypreIJIface.H>


namespace amrex {

class MLNodeLinOp;

class HypreNodeLap
{
public:
    HypreNodeLap (const BoxArray& grids_, const DistributionMapping& dmap_,
                  const Geometry& geom_, const FabFactory<FArrayBox>& factory_,
                  const iMultiFab& owner_mask_, const iMultiFab& dirichlet_mask_,
                  MPI_Comm comm_, MLNodeLinOp const* linop_, int verbose_,
                  const std::string& options_namespace_);
    ~HypreNodeLap ();

    using Int = HYPRE_Int;

    void solve (MultiFab& soln, const MultiFab& rhs,
                Real rel_tol, Real abs_tol, int max_iter);

    void fill_node_id (LayoutData<Int>& offset);

    void setHypreOptionsNamespace(const std::string& ns)
    { options_namespace = ns; }

private:

    BoxArray grids;
    DistributionMapping dmap;
    Geometry geom;
    FabFactory<FArrayBox> const* factory = nullptr;
    iMultiFab const* owner_mask = nullptr;
    iMultiFab const* dirichlet_mask = nullptr;
    MPI_Comm comm = MPI_COMM_NULL;
    MLNodeLinOp const* linop = nullptr;
    int verbose = 0;

    std::unique_ptr<HypreIJIface> hypre_ij;

    // Non-owning references to Hypre matrix, rhs, and solution data
    HYPRE_IJMatrix A = NULL;
    HYPRE_IJVector b = NULL;
    HYPRE_IJVector x = NULL;

    LayoutData<Int> nnodes_grid;
    LayoutData<Vector<Int> > node_id_vec;
    FabArray<BaseFab<Int> > node_id;
    MultiFab tmpsoln;

    std::string options_namespace{"hypre"};

    void loadVectors (MultiFab& soln, const MultiFab& rhs);
    void getSolution (MultiFab& soln);
};

}

#endif

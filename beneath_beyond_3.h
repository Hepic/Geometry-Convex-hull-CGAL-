#ifndef BENEATH_BEYOND_3
#define BENEATH_BEYOND_3

#include <iostream>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/point_generators_3.h>
#include <CGAL/Convex_hull_d.h>
#include <CGAL/Convex_hull_d_traits_3.h>
#include <CGAL/Convex_hull_d_to_polyhedron_3.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Kernel/global_functions.h>
#include <CGAL/Timer.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Modifier_base.h>

using namespace std;

typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
typedef Kernel::Point_3 Point3;
typedef Kernel::Plane_3 Plane3;
typedef CGAL::Creator_uniform_3<double, Point3> Creator;
typedef CGAL::Polyhedron_3<Kernel> Polyhedron3;
typedef Polyhedron3::Vertex_iterator VertexIterator;
typedef Polyhedron3::Facet_iterator FacetIterator;
typedef Polyhedron3::Halfedge_iterator HalfEdgeIterator;
typedef Polyhedron3::Halfedge_handle HalfEdgeHandle;
typedef Polyhedron3::HalfedgeDS HalfedgeDS;
typedef CGAL::Convex_hull_d_traits_3<Kernel> HullTraits3;
typedef CGAL::Convex_hull_d<HullTraits3> ConvexHull3;


struct planeEquation {
    template <class Facet>
    typename Facet::Plane_3 operator()(Facet &f) {
        typename Facet::Halfedge_handle h = f.halfedge();
        typedef typename Facet::Plane_3 Plane;
        return Plane(h->vertex()->point(),
                     h->next()->vertex()->point(),
                     h->next()->next()->vertex()->point());
    }
};

template <class HDS>
class Build_polyHed : public CGAL::Modifier_base<HDS> {
    private:
        Point3 point;
        vector<Point3> borderVertices;
        int vertNum = 4, facetNum = 10; //TODO fix facetNum

    public:
        void operator()(HDS &hds) {
            CGAL::Polyhedron_incremental_builder_3<HDS> B(hds, true);
            B.begin_surface(vertNum, facetNum, 0, CGAL::Polyhedron_incremental_builder_3< HDS >::ABSOLUTE_INDEXING);
            typedef typename HDS::Vertex Vertex;
            typedef typename Vertex::Point Point;
            
            B.add_vertex(point);
            /* B.begin_facet();

            for (FacetIterator facetItr = polyHed.facets_begin(); facetItr != polyHed.facets_end(); ++facetItr) {
                HalfEdgeHandle edge = facetItr->halfedge();
                Point3 pnt1 = edge->vertex()->point();
                Point3 pnt2 = edge->next()->vertex()->point();
                Point3 pnt3 = edge->next()->next()->vertex()->point();

                B.add_vertex_to_facet(pnt1);
                B.add_vertex_to_facet(pnt2);
                B.add_vertex_to_facet(pnt3);
                B.end_facet();
            }
            
            B.end_surface();*/
        }

        void updateParameters(const vector<Point3> &borderVertices, Point3 point) {
            this->borderVertices = borderVertices;
            this->point = point;

            ++this->vertNum;
            this->facetNum *= 2;
        }
};


template <typename Iterator, typename objType>
void beneath_beyond_3(Iterator begin, Iterator end, objType &obj) { 
    // sort points lexicographical
    sort(begin, end);
    
    Iterator itr = begin;
    Iterator initIters[4];
    
    for (int i = 0; i < 4; ++i) {
        initIters[i] = itr;
        ++itr;
    }
    
    // create initial tetrahedron
    obj.make_tetrahedron(*initIters[0], *initIters[1], *initIters[2], *initIters[3]);
    Build_polyHed<HalfedgeDS> builder;
    vector<Point3> borderVertices;

    for (; itr != end; ++itr) {
        // iterate over facets and erase the red vertices, edges, facets
        for (FacetIterator facetItr = obj.facets_begin(); facetItr != obj.facets_end(); ++facetItr) {
            HalfEdgeHandle edge = facetItr->halfedge();
            Point3 pnt1 = edge->vertex()->point();
            Point3 pnt2 = edge->next()->vertex()->point();
            Point3 pnt3 = edge->next()->next()->vertex()->point();
            
            cout << pnt1 << " " << pnt2 << " " << pnt3 << " compare with " << *itr << " == ";
            cout << CGAL::orientation(pnt1, pnt2, pnt3, *itr) << endl; 
            
            // check orientation of points
            if (CGAL::orientation(pnt1, pnt2, pnt3, *itr) == 1) {
                obj.erase_facet(edge); 
            }
        }
        
        // find border vertices to add the new facets
        borderVertices.clear();

        for (HalfEdgeIterator edgeItr = obj.halfedges_begin(); edgeItr != obj.halfedges_end(); ++edgeItr) {
            if (edgeItr->is_border()) {
                borderVertices.push_back(edgeItr->vertex()->point());
                HalfEdgeIterator tempItr = edgeItr->next();
                
                // add all distinct border vertices
                while (edgeItr->vertex()->point() != tempItr->vertex()->point()) {
                    borderVertices.push_back(edgeItr->vertex()->point());
                    tempItr = tempItr->next();
                }
                
                break;
            }
        }

        builder.updateParameters(borderVertices, *itr);
        //obj.delegate(builder);
    }
}

#endif

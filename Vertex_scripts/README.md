# Vertex scripts
## z_res_mc.C
This can be used to calculate the z vertex resolution from the difference between thrown and reconstructed track z vertices in simulated data. It makes a plot of vertex difference vs thrown vertex and then fits the difference with a gaussian to find the std deviation.  This is done for several large chunks of z, to show any variation along the length of the target, and then repeated at each end of the target, using a chunk of length dz, to find the resolution close to each window.

See [GlueX-doc-4924](https://halldweb.jlab.org/DocDB/0049/004924/002/TaskForce_ProtonFiducialVertex.pdf) 

The arguments are the filename, the treename, the vertex positions (cm) and dz (cm). Sensible defaults are provided for the last three.

```sh
root 'z_res_mc.C("tree_mc_phi_31057.root","kpkm__B4_Tree",50.3,79.8,1)'
```

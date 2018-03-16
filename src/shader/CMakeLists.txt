install(FILES
  mesh.frag mesh.vert solution.frag solution.vert
  cross.vert cross.frag
  clipping.vert clipping.geom vector.geom isosurface.geom elements.geom elements.frag elements.vert utils.inc interpolation.inc
  font.vert font.frag font.geom tess.tesc tess.tese
  DESTINATION ngsolve/gui/shader
)

vtk_module(vtkFiltersParallelGeometry
  IMPLEMENTS
    vtkFiltersParallel
  GROUPS
    MPI
  TEST_DEPENDS
    vtkIOXML
    vtkIOParallel
    vtkIOParallelXML
    vtkCommonDataModel
    vtkParallelMPI
    vtkTestingCore
    vtkImagingCore
  KIT
    vtkParallel
  DEPENDS
    vtkCommonCore
    vtkCommonExecutionModel
    vtkFiltersGeometry
    vtkFiltersParallel
    vtkParallelMPI
  PRIVATE_DEPENDS
    vtkCommonDataModel
    vtkFiltersExtraction
    vtkFiltersGeneral
    vtkIOLegacy
    vtkParallelCore
  )

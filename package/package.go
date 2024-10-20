package crendergraph

import (
	denv "github.com/jurgen-kluft/ccode/denv"
	cdag "github.com/jurgen-kluft/cdag/package"
	cunittest "github.com/jurgen-kluft/cunittest/package"
)

// GetPackage returns the package object of 'crendergraph'
func GetPackage() *denv.Package {
	// Dependencies
	cunittestpkg := cunittest.GetPackage()
	cdagpkg := cdag.GetPackage()

	// The main (crendergraph) package
	mainpkg := denv.NewPackage("crendergraph")
	mainpkg.AddPackage(cunittestpkg)
	mainpkg.AddPackage(cdagpkg)

	// 'crendergraph' library
	mainlib := denv.SetupDefaultCppLibProject("crendergraph", "github.com\\jurgen-kluft\\crendergraph")
	mainlib.Dependencies = append(mainlib.Dependencies, cdagpkg.GetMainLib())

	// 'crendergraph' unittest project
	maintest := denv.SetupDefaultCppTestProject("crendergraph"+"_test", "github.com\\jurgen-kluft\\crendergraph")
	maintest.Dependencies = append(maintest.Dependencies, cunittestpkg.GetMainLib())
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}

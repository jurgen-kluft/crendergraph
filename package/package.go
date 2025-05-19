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
	cgfxpkg := cdag.GetPackage()
	cdagpkg := cdag.GetPackage()

	// The main (crendergraph) package
	mainpkg := denv.NewPackage("crendergraph")
	mainpkg.AddPackage(cunittestpkg)
	mainpkg.AddPackage(cgfxpkg)
	mainpkg.AddPackage(cdagpkg)

	// 'crendergraph' library
	mainlib := denv.SetupCppLibProject("crendergraph", "github.com\\jurgen-kluft\\crendergraph")
	mainlib.AddDependencies(cgfxpkg.GetMainLib()...)
	mainlib.AddDependencies(cdagpkg.GetMainLib()...)

	// 'crendergraph' unittest project
	maintest := denv.SetupDefaultCppTestProject("crendergraph"+"_test", "github.com\\jurgen-kluft\\crendergraph")
	maintest.AddDependencies(cunittestpkg.GetMainLib()...)
	maintest.Dependencies = append(maintest.Dependencies, mainlib)

	mainpkg.AddMainLib(mainlib)
	mainpkg.AddUnittest(maintest)
	return mainpkg
}

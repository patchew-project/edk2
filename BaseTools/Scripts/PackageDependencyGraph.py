# @file
# Recursively scan a directory for EDK II packages and generate GraphViz dot
# input that is used to render a graph of package dependencies in SVG format.
#
# Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

import os
import sys
import argparse
import subprocess
import webbrowser
import networkx as nx
from edk2toollib.uefi.edk2.parsers.inf_parser import InfParser

#
# Globals for help information
#
__prog__        = 'PackageDependencyGraph'
__copyright__   = 'Copyright (c) 2019, Intel Corporation. All rights reserved.'
__description__ = '''Recursively scan a directory for EDK II packages and
generate GraphViz dot input that is used to render a graph of package
dependencies in SVG format.'''

if __name__ == '__main__':

    #
    # Create command line argument parser object
    #
    parser = argparse.ArgumentParser (prog = __prog__,
                                      description = __description__ + '\n' + __copyright__,
                                      conflict_handler = 'resolve')
    parser.add_argument ("-w", "--workspace", dest = 'Workspace', default = os.curdir,
                         help = "Directory to recursively scan for EDK II packages.  Default is current directory.")
    parser.add_argument ("-p", "--packages-path", dest = 'PackagesPath', default = None,
                         help = "List of directories to recursively scan for EDK II packages.")
    parser.add_argument ("-d", "--dot-output", dest = 'DotOutputFile',
                         help = "DOT output filename.")
    parser.add_argument ("-o", "--output", dest = 'SvgOutputFile',
                         help = "SVG output filename.")
    parser.add_argument ("-g", "--ignore-directory", dest = 'IgnoreDirectory', action='append', default=[],
                         help = "Name of directory to ignore.  Option can be repeated to ignore multiple directories.")
    parser.add_argument ("-k", "--skip-package", dest = 'SkipPackage', action='append', default=[],
                         help = "Name of EDK II Package DEC file to skip.  Option can be repeated to skip multiple EDK II packages.")
    parser.add_argument ("-s", "--self-dependency", dest = 'SelfDependency', action = "store_true", default = False,
                         help = "Include self links in dependency graph.  Default is disabled.")
    parser.add_argument ("-u", "--unresolved", dest = 'Unresolved', action = "store_true", default=False,
                         help = "Include unresolved EDK II packages in dependency graph.  Default is disabled.")
    parser.add_argument ("-l", "--label", dest = 'Label', action = "store_true", default=False,
                         help = "Label links with the number of EDK II package dependencies.  Default is disabled.")
    parser.add_argument ("-f", "--full-paths", dest = 'FullPaths', action = "store_true", default=False,
                         help = "Label package nodes with full path to EDK II package.  Default is disabled.")
    parser.add_argument ("-b", "--web-browser", dest = 'WebBrowser', action = "store_true", default=False,
                         help = "Display SVG output file in default web browser.  Default is disabled.")
    parser.add_argument ("-v", "--verbose", dest = 'Verbose', action = "store_true",
                         help = "Increase output messages")
    parser.add_argument ("-q", "--quiet", dest = 'Quiet', action = "store_true",
                         help = "Reduce output messages")
    parser.add_argument ("--debug", dest = 'Debug', type = int, metavar = '[0-9]', choices = range (0, 10), default = 0,
                         help = "Set debug level")

    #
    # Parse command line arguments
    #
    args = parser.parse_args ()

    #
    # Find all EDK II package DEC files
    #
    Components = {}
    SearchPaths = [args.Workspace]
    if args.PackagesPath:
        SearchPaths += args.PackagesPath.split(os.pathsep)
    SearchPaths = [os.path.realpath(x) for x in SearchPaths]
    for SearchPath in SearchPaths:
        for root, dirs, files in os.walk (SearchPath):
            for name in files:
                FilePath = os.path.join (root, name)
                if set(FilePath.split(os.sep)).intersection(set(args.IgnoreDirectory)) != set():
                    if args.Verbose:
                        print ('IGNORE:' + FilePath)
                    continue
                if os.path.splitext(FilePath)[1].lower() in ['.dec']:
                    DecFile = os.path.realpath (FilePath)
                    if os.path.split(DecFile)[1] in args.SkipPackage:
                        if args.Verbose:
                            print ('SKIP:' + DecFile)
                        continue
                    if DecFile not in Components:
                        if args.Verbose:
                            print ('PACKAGE:' + DecFile)
                        Components[DecFile] = {}

    #
    # Find EDK II component INF files in each EDK II package
    #
    PackageLabels = {}
    UnresolvedPackages = []
    AmbiguousDependencies = []
    for DecFile in Components:
        DecPath = os.path.split (DecFile)[0]
        if DecFile not in PackageLabels:
            PackageLabels[DecFile] = (DecFile.replace(os.path.sep,'\\n'), 'white')
            if not args.FullPaths:
                PackagePath = os.path.relpath(DecFile, os.path.split(DecPath)[0])
                PackageLabels[DecFile] = (PackagePath.replace(os.path.sep,'\\n'), 'white')
        for root, dirs, files in os.walk (DecPath):
            for name in files:
                FilePath = os.path.join(root, name)
                if set(FilePath.split(os.sep)).intersection(set(args.IgnoreDirectory)) != set():
                    if args.Verbose:
                        print ('IGNORE:' + FilePath)
                    continue
                if os.path.splitext(FilePath)[1].lower() in ['.inf']:
                    InfFile = os.path.realpath (FilePath)
                    Inf = InfParser ()
                    Inf.ParseFile (InfFile)
                    DependentPackages = []
                    for Dependency in Inf.PackagesUsed:
                        Dependency = os.path.normpath(Dependency)
                        if os.path.split(Dependency)[1] in args.SkipPackage:
                            if args.Verbose:
                                print ('SKIP:' + Dependency)
                            continue
                        Found = False
                        for SearchPath in SearchPaths:
                            PackagePath = os.path.realpath(os.path.join(SearchPath, Dependency))
                            if os.path.exists(PackagePath):
                                DependentPackages.append(PackagePath)
                                if not args.FullPaths:
                                    PackageLabels[PackagePath] = (Dependency.replace(os.path.sep,'\\n'), 'white')
                                Found = True
                                break
                        if not Found:
                            Count = 0
                            Match = ''
                            for DecFile2 in Components:
                                if DecFile2.endswith(Dependency):
                                    if Count == 0:
                                        Match = DecFile2
                                    Count = Count + 1
                            if Count > 1:
                                AmbiguousDependencies.append (Dependency)
                            if Count == 1:
                                DependentPackages.append(Match)
                                if not args.FullPaths:
                                    PackageLabels[Match] = (Dependency.replace(os.path.sep,'\\n'), 'white')
                                Found = True
                        if not Found and args.Unresolved:
                            if args.Verbose:
                                print ('WARNING: Dependent package not found ' + Dependency)
                            DependentPackages.append(Dependency)
                            UnresolvedPackages.append(Dependency)
                            PackageLabels[Dependency] = (Dependency.replace(os.path.sep,'\\n'), 'white')
                    Components[DecFile][InfFile] = DependentPackages
    if AmbiguousDependencies:
        for Dependency in set(AmbiguousDependencies):
            print ('ERROR: MULTIPLE:   ' + Dependency)
        print ('Use --packages-path to provide search priority.')
        sys.exit(1)

    #
    # Generate GraphViz dot input file contents.
    # Use networkx to detect circular dependencies.
    #
    MaxWeight = 0
    MaxWeightDecFile = list(Components.keys())[0]
    Graph = nx.DiGraph()
    Edges = []
    for DecFile in Components:
        if args.Verbose:
            print ('PACKAGE DEPENDENCIES:' + DecFile)
        AllDependencies = []
        Dependencies = set()
        for InfFile in Components[DecFile]:
            AllDependencies += Components[DecFile][InfFile]
            Dependencies = Dependencies.union(Components[DecFile][InfFile])
        if not args.SelfDependency:
            Dependencies = Dependencies.difference(set([DecFile]))
        for Dependency in Dependencies:
            Weight = AllDependencies.count(Dependency)
            if Weight > MaxWeight:
                MaxWeight = Weight
                MaxWeightDecFile = DecFile
            if args.Verbose:
                print ('  DEPENDENCY: Weight(' + str(Weight) + ') ' + Dependency)
            Edges.append('  "{Package}" -> "{Dependency}" [label = "{Weight}"];'.format(
                Package    = DecFile,
                Dependency = Dependency,
                Weight     = str(Weight) if args.Label else ''
                ))
            Graph.add_edge(DecFile, Dependency)
    Edges.sort()

    #
    # Set fill color to yellow if a packages is part of a circular dependency
    #
    for Node in set([x for y in list(nx.simple_cycles(Graph)) for x in y]):
        PackageLabels[Node] = (PackageLabels[Node][0], 'yellow')
        print ('ERROR: CIRCULAR:   ' + Node)

    #
    # Set fill color to pink if a package that is nested inside another package.
    # Set fill color to orange if a package is nested inside another package and
    # is part of a circular dependency.
    #
    for DecFile in Components:
        DecPath = os.path.split (DecFile)[0]
        for DecFile2 in Components:
            if DecFile2 == DecFile:
                continue
            if os.path.commonpath ([DecPath, DecFile2]) != DecPath:
                continue
            if len(DecFile2) < len(DecPath):
                DecFile2 = DecFile
            print ('ERROR: NESTED:     ' + DecFile2)
            if PackageLabels[DecFile2][1] == 'yellow':
                PackageLabels[DecFile2] = (PackageLabels[DecFile2][0], 'orange')
            else:
                PackageLabels[DecFile2] = (PackageLabels[DecFile2][0], 'pink')

    #
    # Set fill color to red for nodes that are unresolved
    #
    for Node in set(UnresolvedPackages):
        PackageLabels[Node] = (PackageLabels[Node][0], 'red')
        print ('ERROR: UNRESOLVED: ' + Node)

    #
    # Add node statements to set node label and fill color
    #
    Nodes = []
    for Package in PackageLabels:
        Nodes.append('  "{Package}" [label="{Label}",fillcolor={Color}];'.format(
            Package = Package,
            Label = PackageLabels[Package][0],
            Color = PackageLabels[Package][1]
            ))
    Nodes.sort()

    #
    # Generate dot file from Nodes and Edges and add a Legend at top of graph
    #
    Dot = []
    Dot.append('digraph {')
    Dot.append('  rankdir=BT;')
    Dot.append('  node [shape=Mrecord,style=filled];')
    Dot.append('')
    Dot = Dot + Nodes
    Dot.append('')
    Dot = Dot + Edges
    Dot.append('')
    Dot.append('  subgraph legend {')
    Dot.append('    rank=sink;')
    Dot.append('    Unresolved     [label="Unresolved Dependency",                  fillcolor=red];')
    Dot.append('    Circular       [label="Circular Dependency",                    fillcolor=yellow];')
    Dot.append('    Nested         [label="Nested Package",                         fillcolor=pink];')
    Dot.append('    NestedCircular [label="Nested Package with Circular Dependency",fillcolor=orange];')
    Dot.append('  }')
    if MaxWeightDecFile:
        Dot.append('  Unresolved->"' + MaxWeightDecFile + '" [style=invis];')
    Dot.append('}')

    if args.DotOutputFile:
        #
        # Write GraphViz dot file contents to DotOutputFile
        #
        with open(os.path.realpath(args.DotOutputFile), 'w') as File:
            File.write('\n'.join(Dot))
    if args.SvgOutputFile:
        #
        # Use GraphViz 'dot' command to generate SVG output file
        #
        args.SvgOutputFile = os.path.realpath(args.SvgOutputFile)
        try:
            Process = subprocess.Popen('dot -Tsvg',
              stdin=subprocess.PIPE,
              stdout=open(args.SvgOutputFile, 'w'),
              stderr=subprocess.PIPE,
              shell=True
              )
            Process.stdin.write ('\n'.join(Dot).encode())
            Process.communicate()
            if Process.returncode != 0:
                print ("ERROR: Can not run GraphViz 'dot' command.  Check install and path.")
                sys.exit(Process.returncode)
        except:
            print ("ERROR: Can not run GraphViz 'dot' command.  Check install and path.")
            sys.exit(1)
        #
        # Display SVG file in default web browser
        #
        if args.WebBrowser:
            webbrowser.open(args.SvgOutputFile)

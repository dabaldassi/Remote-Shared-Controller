#include <iostream>

#include <rsccli.hpp>

int main(int argc, char *argv[])
{
  rscui::RSCCli cli;
  
  return cli.run(argc,argv);
}

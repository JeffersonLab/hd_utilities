/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <iostream>

#include "wave.h"

wave::wave(const wave& o)
{
  l = o.l;
  m = o.m;
  name = o.name;
  idx = o.idx;
  phaseLocked = o.phaseLocked;
}


waveset::waveset() { return; }

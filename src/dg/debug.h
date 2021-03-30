#pragma once


int dg_open(const char* path);
int dg_close();

void dg_log(const char* msg);
void dg_wrn(const char* msg);
void dg_err(const char* msg);


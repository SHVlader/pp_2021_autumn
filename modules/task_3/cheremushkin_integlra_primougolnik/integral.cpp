// Copyright 2021 Cheremushkin Kirill
#include "../../../modules/task_3/cheremushkin_integlra_primougolnik/integral.h"
#include <vector>
#include <mpi.h>
double linelintegral(const std::vector<std::pair<double, double>>& limits)
{
	int i;
	int m = 0 ;
	int master = 0;
	int n = 0;
	int process;
	int process_num;
	double q_global = 0.0;
	double q_local =0.0;
	MPI_Comm_size(MPI_COMM_WORLD, &process_num);
	double x, y, z;
	std::vector<double>xb(limits.size() * 2);
	double x_max = limits[0].second;
	double x_min = limits[0].first;
	for (process = 1; process <= process_num - 1; process++)
	{
		int i = 0;
		int c = 0;
		for (int j = 0; j < limits.size(); j++)
		{
			x_max = limits[j].second;
			x_min = limits[j].first;
			c = 0;
			while (2 != c)
			{
				if (i % 2 == 0)
				{
					xb[i] = ((double)(process_num - process) * x_min + (double)(process - 1) * x_max) / (double)(process_num - 1);
				}
				else
				{
					xb[i] = ((double)(process_num - process - 1) * x_min + (double)(process)*x_max) / (double)(process_num - 1);
				}
				c++;
				i++;
			}
		}
	}
		q_local = 0.0;
		m = 100;
		double mass[3];
		for (i = 1; i <= m; i++)
		{
			if (xb.size() / 2 == 1)
			{
				x = ((double)(2 * m - 2 * i + 1) * xb[0] + (double)(2 * i - 1) * xb[1]) / (double)(2 * m);
				q_local = q_local + f(x);
			}
			if (xb.size() / 2 == 2)
			{
				x = ((double)(2 * m - 2 * i + 1) * xb[0] + (double)(2 * i - 1) * xb[1]) / (double)(2 * m);
				y = ((double)(2 * m - 2 * i + 1) * xb[2] + (double)(2 * i - 1) * xb[3]) / (double)(2 * m);
				mass[0] = x; mass[1] = y;
				q_local = q_local + ff(mass);
			}
			if (xb.size() / 2 == 3)
			{
				x = ((double)(2 * m - 2 * i + 1) * xb[0] + (double)(2 * i - 1) * xb[1]) / (double)(2 * m);
				y = ((double)(2 * m - 2 * i + 1) * xb[2] + (double)(2 * i - 1) * xb[3]) / (double)(2 * m);
				z = ((double)(2 * m - 2 * i + 1) * xb[4] + (double)(2 * i - 1) * xb[5]) / (double)(2 * m);
				mass[0] = x; mass[1] = y; mass[2] = z;
				q_local = q_local + fff(mass);
			}
		}
		if (xb.size() / 2 == 1) {

			q_local = q_local * (xb[1] - xb[0]) / (double)(m);
		}
		if (xb.size() / 2 == 2) {
			q_local = q_local * (xb[1] - xb[0]) * (xb[3] - xb[2]) / (double)(m) / (double)(m);
		}
		if (xb.size() / 2 == 3) {
			q_local = q_local * (xb[1] - xb[0]) / (double)(m) * (xb[3] - xb[2]) / (double)(m) * (xb[5] - xb[4]) / (double)(m);
		}
		q_global = 0.0;
		q_global = q_global + q_local;
		return q_global - 0.624;
}
double Parallelintegral(const std::vector<std::pair<double, double>>& limits)
{
	int i;
	int ierr;
	int m;
	int master = 0;
	int n;
	int process;
	int process_id;
	int process_num;
	double q_global = 0.0;
	double q_local = 0.0;
	int received;
	int source;
	MPI_Status status;
	int tag;
	int target;
	double x, y, z;
	std::vector<double>xb(limits.size() * 2);
	double x_max = limits[0].second;
	double x_min = limits[0].first;
	ierr = MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
	ierr = MPI_Comm_size(MPI_COMM_WORLD, &process_num);
	if (process_id == master)
	{
		if (process_num <= 1)
		{
			ierr = MPI_Finalize();
			exit(1);
		}
	}
	if (process_id == master)
	{
		for (process = 1; process <= process_num - 1; process++)
		{
			target = process;
			tag = 1;
			int i = 0;
			int c = 0;
			for (int j = 0; j < limits.size(); j++)
			{
				x_max = limits[j].second;
				x_min = limits[j].first;
				c = 0;
				while (2 != c)
				{
					if (i % 2 == 0)
					{
						xb[i] = ((double)(process_num - process) * x_min + (double)(process - 1) * x_max) / (double)(process_num - 1);
					}
					else
					{
						xb[i] = ((double)(process_num - process - 1) * x_min + (double)(process)*x_max) / (double)(process_num - 1);
					}
					ierr = MPI_Send(&xb[i], 2, MPI_DOUBLE, target, tag, MPI_COMM_WORLD);
					c++;
					i++;
				}
			}
		}
	}
	else
	{
		source = master;
		tag = 1;
		for (int i = 0; i < xb.size(); i++)
		{
			ierr = MPI_Recv(&xb[i], 2, MPI_DOUBLE, source, tag, MPI_COMM_WORLD, &status);
		}
	}
	ierr = MPI_Barrier(MPI_COMM_WORLD);
	m = 100;
	source = master;
	ierr = MPI_Bcast(&m, 1, MPI_INT, source, MPI_COMM_WORLD);
	if (process_id != master)
	{
		q_local = 0.0;
		double mass[3];
		for (i = 1; i <= m; i++)
		{
			if (xb.size() / 2 == 1)
			{
				x = ((double)(2 * m - 2 * i + 1) * xb[0] + (double)(2 * i - 1) * xb[1]) / (double)(2 * m);
				q_local = q_local + f(x);
			}
			if (xb.size() / 2 == 2)
			{
				x = ((double)(2 * m - 2 * i + 1) * xb[0] + (double)(2 * i - 1) * xb[1]) / (double)(2 * m);
				y = ((double)(2 * m - 2 * i + 1) * xb[2] + (double)(2 * i - 1) * xb[3]) / (double)(2 * m);
				mass[0] = x; mass[1] = y;
				q_local = q_local + ff(mass);
			}
			if (xb.size() / 2 == 3)
			{
				x = ((double)(2 * m - 2 * i + 1) * xb[0] + (double)(2 * i - 1) * xb[1]) / (double)(2 * m);
				y = ((double)(2 * m - 2 * i + 1) * xb[2] + (double)(2 * i - 1) * xb[3]) / (double)(2 * m);
				z = ((double)(2 * m - 2 * i + 1) * xb[4] + (double)(2 * i - 1) * xb[5]) / (double)(2 * m);
				mass[0] = x; mass[1] = y; mass[2] = z;
				q_local = q_local + fff(mass);
			}
		}
		if (xb.size() / 2 == 1) {

			q_local = q_local * (xb[1] - xb[0]) / (double)(m);
		}
		if (xb.size() / 2 == 2) {
			q_local = q_local * (xb[1] - xb[0]) * (xb[3] - xb[2]) / (double)(m) / (double)(m);
		}
		if (xb.size() / 2 == 3) {
			q_local = q_local * (xb[1] - xb[0]) / (double)(m) * (xb[3] - xb[2]) / (double)(m) * (xb[5] - xb[4]) / (double)(m);
		}
		target = master;
		tag = 2;
		ierr = MPI_Send(&q_local, 1, MPI_DOUBLE, target, tag, MPI_COMM_WORLD);
	}
	else
	{
		received = 0;
		q_global = 0.0;
		while (received < process_num - 1)
		{
			source = MPI_ANY_SOURCE;
			tag = 2;
			ierr = MPI_Recv(&q_local, 1, MPI_DOUBLE, source, tag, MPI_COMM_WORLD,
				&status);
			q_global = q_global + q_local;
			received = received + 1;
		}
	}
	return q_global - 0.624;
}
double f(double x)
{
	double value;
	value = 2.0 * cos(x) / (2.0 + x * x);
	return value;
}
double ff(double* xx)
{
	double value;
	double x = xx[0];
	double y = xx[1];
	value = x + y * y;
	return value;
}
double fff(double* xx)
{
	double value;
	double x = xx[0];
	double y = xx[1];
	double z = xx[2];
	value = sin(x) * y * z * z;
	return value;
}


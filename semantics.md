# Android-Multitasking-Mechanism
- $realact(S) = A$, where $S$ is created by an action $start(A,F)$
- $tact(S) = A_1$,  $bact(S) = A_m$. %where S = A(act^*)$



\begin{itemize}
	%	\item $\realact(S) = A$, where $S$ is created by an action $start(A,\Ff)$
	\item $\tact(S) = A_1$,  $\bact(S) = A_m$. %~\mbox{where}~ S = A(\act^*)$
	%	\item.  %, ~\mbox{where}~ S = (\act^*)A$\push
	\item $\toptsk(\rho) = S_1$,  %this is the foreground task
	$\topact(\rho) = \tact(\toptsk(\rho))$. %, where $S_1 = [A]\cdot S_1'$.
	
	\item $\push((\rho, \ell), B) = 
	(((([B] \cdot S_1),A_1),(S_2,A_2), \cdots, (S_n,A_n)), \ell)$.
	
	\item $
	\mvacttop((\rho, \ell), B)  =   ((([B]\cdot S'_1 \cdot S''_1), (S_2,A_2), \cdots, (S_n,A_n)), \ell)
	$, if	$S_1=S'_1 \cdot[B]\cdot S''_1$ with $S'_1 \in (\act \setminus \{B\})^*$.
	%---------------------------------------------------------------------------------------------------------------
	
	\item $\clrtop((\rho, \ell), B) = (((S_1'',A_1),(S_2,A_2),\cdots, (S_n,A_n)), \ell)$ if $S_1=S'_1 \cdot S''_1$ with $S'_1 \in (\act \setminus \{B\})^*B$, 
	
	\item $\clrtsk((\rho, \ell))  = ((([],A_1),(S_2,A_2),\cdots, (S_n,A_n)), \ell)$,
	
	\item $\mvtsktop((\rho, \ell), S_i) = (((S_i,A_i),(S_1,A_1),\cdots, (S_{i-1},A_{i-1}), (S_{i+1},A_{i+1}),\cdots, (S_n,A_n)), \ell'),$
	where
	$\ell' = 
	\left\{ 
	\begin{array}{l c l}
	0 & & \mbox{ if } \ell =0,\\
	1 & & \mbox{ if } \ell = i,\\
	\ell & & \mbox{ if } i+1 \le \ell \le n,\\
	\ell+1 & & \mbox{ if } 1 \le \ell \le i-1.
	\end{array}
	\right.$ 
	
	[Note that $\ell'$ is the simply the new position of the main task.]
	\item $\newtsk((\rho, \ell), B)  =  
	((([B],B),(S_1,A_1),\cdots, (S_n,A_n)), \ell')$, 
	where $\ell' = 0$ if $\ell=0$, and $\ell' = \ell+1$ otherwise.
	%---------------------------------------------------------------------------------------------------------------
	
	\item $\getrealtsk(\rho, B) = S_i$ such that $i \in [n]$ is the \emph{minimum} index satisfying $A_i = B$ if such an index $i$ exists; $\getrealtsk(\rho, B) = *$ otherwise.
	%---------------------------------------------------------------------------------------------------------------
	
	\item $\gettsk(\rho, B) = S_i$ such that $i \in [n]$ is the \emph{minimum} index satisfying $\aft(A_i)=\aft(B)$, if such an index $i$ exists; $\gettsk(\rho, B) = *$ otherwise.
	
	\item $\rmact((\rho, \ell), i) = 
	\left\{ 
	\begin{array}{l l}
		(0, (((S_1, A_1), \cdots, (S_{i-1}, A_{i-1}), (S_{i+1}, A_{i+1}), \cdots, (S_n, A_n)), 0))  & \mbox{ if } m=1  \mbox{ and }\ell = 0 \mbox{ or } i,\\
		(0, (((S_1, A_1), \cdots, (S_{i-1}, A_{i-1}), (S_{i+1}, A_{i+1}), \cdots, (S_n, A_n)), \ell))  & \mbox{ if } m=1  \mbox{ and }1 \le \ell \le i-1,\\
		(0, (((S_1, A_1), \cdots, (S_{i-1}, A_{i-1}), (S_{i+1}, A_{i+1}), \cdots, (S_n, A_n)), \ell-1))  & \mbox{ if } m=1  \mbox{ and }i+1 \le \ell \le n,\\
		(i, (((S_1, A_1), \cdots, (S_{i-1}, A_{i-1}), ([B_2,\cdots, B_m],A_i), (S_{i+1},A_{i+1}),  \cdots, (S_n, A_n)), \ell))  & \mbox{ if } m > 1.
	\end{array}
	\right.$
	
	
	%
	%-------------------------------------------------------------------------------------------------
\end{itemize}

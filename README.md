# User Guide

## Overview
    This repository contains VMR (Versal Management Runtime) firmware. The
    firmware contains management software for Versal based hardware, such as
    card management and card seasor reporting. It is corss-compiled by ARM-R5 C
    compiler and running as a FreeRTOS application.

## Code Review procedure

### 1. Code Review Checklist (for all developers who can submit PRs)

        1.1 Will this change potentially stop VMR running? 
            > If yes, please explain more details.

	1.2 Is coding style aligned with FreeRTOS code style?

	1.3 A CR or Story number is required for every PR. And PR # should be
	    updated back to CR/Story.
	    > PR reviewer should not apporve any PR without a tracking number

        1.4 Please address all code review comments prior to ask for code merge.

>       Failing to do so might end up with "git revert" due to "need more work".
	
	
### 2. Code Merge Checklist (for maintainers who can merge the code to gate)

        2.1 Is there a unit test report?
            > unit test should be performed on TA XRT + TA Shell + TA APU;

        2.2 Are there 2+ code reviewers approved the PR on github?

>       Failing to do so might end up with "git revert" if serious regression found. 

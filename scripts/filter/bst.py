# copied from http://www.cs.toronto.edu/~rdanek/csc148h_09
class BinarySearchTree:
	def __init__(self,lenght):
		""" create an empty binary search tree """
		self.tree = [None for i in range(lenght)]
		self.count = 0
		
	def put(self, line, lon):
		""" add a new mapping between key and value to the BST """
		self.count+=1
		if self.tree[1]==None:
			self.tree[1]=line
			return (None,None)
		else:
			lcmp = line.compare(lon)
			idx = 1
			while(self.tree[idx].compare(lon) != lcmp):
				if lcmp <  self.tree[idx].compare(lon):
					if self.tree[idx*2]:
						idx*=2
						continue
					else:
						self.tree[idx*2] = line
						return (self.tree[idx],self.getNext(idx*2))
				else:
					if self.tree[idx*2+1]:
						idx = idx*2+1
						continue
					else:
						self.tree[idx*2+1] = line
						return (self.getPrev(idx*2+1),self.tree[idx])
			return (self.getPrev(idx),self.getNext(idx))
	
	def getPrev(self,idx):
		if self.tree[idx*2]:
			return self.tree[self._findMax(idx*2)]
		if idx==1:
			return None
		memidx = idx
		idx /= 2
		while (idx%2==0):
			memidx = idx
			idx/=2
		if idx*2 == memidx:
			return self.tree[idx]
		return None
	def getNext(self,idx):
		try:
			if self.tree[idx*2+1]:
				return self.tree[self._findMin(idx*2+1)]
		except IndexError:
			pass
			#self.tree.extend([None for x in range(len(self.tree))])
		if idx==1:
			return None
		memidx = idx
		idx /= 2
		while (idx%2==1):
			memidx = idx
			idx/=2
		if idx*2+1 == memidx:
			return self.tree[idx]
		return None
		

	def get(self, line, lon):
		""" retrieve the value associated with the given key """
		if self.tree[1]:
			lcmp = line.compare(lon)
			idx = 1
			while ( self.tree[idx] != None and self.tree[idx] != line):
				if lcmp < self.tree[idx].compare(lon):
					idx = idx*2
				else:
					idx = idx*2 + 1
			return self.tree[idx]
		else:
			return None
	
	def _findMin(self,idx):
		while( self.tree[idx*2]):
			idx*=2
		return idx
	def _findMax(self,idx):
		while( self.tree[idx*2+1]):
			idx = idx*2+1
		return idx
		
	def delete(self, line, lon):
		""" delete the node with the given key if it exists """
		self.count -=1
		if self.tree[1]:
			lcmp = line.compare(lon)
			idx = 1
			tcmp = self.tree[1].compare(lon)
			while( tcmp != lcmp):
				if lcmp < tcmp:
					if self.tree[idx*2]:
						idx*=2
						tcmp = self.tree[idx].compare(lon)
					else: 
						print "Not found!"
						return None
				else:
					if self.tree[idx*2+1]:
						idx = idx*2+1
						tcmp = self.tree[idx].compare(lon)
					else:
						print "Not found!"
						return None
			if self.tree[idx]!=line:
				print "Removing wrong"
			if self.tree[idx*2+1] and self.tree[idx*2+1]:
				succ = self._findMin(idx*2+1)
				psucc = succ/2
				if psucc*2 == succ:
					self.tree[psucc*2]=self.tree[succ*2+1]
				self.tree[succ*2]=self.tree[idx*2]
				self.tree[succ*2+1]=self.tree[idx*2+1]
				self.tree[idx] =  self.tree[succ]
			else:
				if self.tree[idx*2]:
					self.tree[idx] = self.tree[idx*2]
				if self.tree[idx*2+1]:
					self.tree[idx] = self.tree[idx*2+1]
					

			
class TreeNode:
	def __init__(self, line):
		self.line = line
		self.left = None
		self.right = None 
		self.parent = None
			
	def put(self, line, lcmp, lon):
		""" add a new mapping between key and value in the tree """
		if self.line.compare(lon) == lcmp:
		#	print "Warning, identical lines!"
			return (self.getPrev(),self.getNext())
		elif self.line.compare(lon) > lcmp:			# key belongs in left subtree 
			if self.left:
				return self.left.put(line, lcmp,lon)
			else:					   # left subtree is empty
				self.left = TreeNode(line)
				self.left.parent = self
				return (self.left.getPrev(),self.left.getNext())
		else:						   # key belongs in right subtree 
			if self.right:
				return self.right.put(line,lcmp,lon)
			else:					   # right subtree is empty
				self.right = TreeNode(line)
				self.right.parent = self
				return (self.right.getPrev(),self.right.getNext())
				
	def getPrev(self):
		if self.left:
			return self._findMax(self)[1].line
		if not self.parent:
			return None

		memself = self
		self = self.parent
		while (self.parent and self.parent.left==self):
			self = self.parent

		if self.right==memself:
			return self.line
		return None

	def getNext(self):
		if self.right:
			return self._findMin(self)[1].line
		if not self.parent:
			return None

		memself = self
		self = self.parent
		while (self.parent and self.parent.right==self):
			self = self.parent

		if self.left==memself:
			return self.line
		return None
		
		
		
	def get(self, line,lcmp,lon):
		""" get the value associated with the key """
		if self.line == line:
			return self.line
		
		if self.line.compare(lon) > lcmp:	   # key should be in the left subtree
			if self.left:
				return self.left.get(line,lcmp,lon)
			else:
				return None
		else:						   # key should be in the right subtree
			if self.right:
				return self.right.get(line,lcmp,lon)
			else:
				return None
			
	def delete(self, line,lcmp,lon):
		""" delete the node with the given key and return the 
		root node of the tree """
			
		if self.line.compare(lon) == lcmp:
			if self.line != line:
				pass
			#   print "Warning, non equal objects!"	
			# found the node we need to delete
			
			if self.right and self.left: 
				
				# get the successor node and its parent 
				[psucc, succ] = self.right._findMin(self)
				
				# splice out the successor
				# (we need the parent to do this) 
				
				if psucc.left == succ:
					psucc.left = succ.right
					if succ.right:
						psucc.left.parent = psucc
				else:
					psucc.right = succ.right
					if succ.right:
						psucc.right.parent = succ.right
								
				# reset the left and right children of the successor
				
				succ.left = self.left
				if self.left:
					succ.left.parent = succ
				succ.right = self.right
				if self.right:
					succ.right.parent = succ
				
				return succ				
				
			else:
				# "easier" case
				if self.left:
					return self.left	# promote the left subtree
				else:
					return self.right   # promote the right subtree 
		else:
			if self.line.compare(lon) > lcmp:		  # key should be in the left subtree
				if self.left:
					self.left = self.left.delete(line,lcmp,lon)
					if self.left:
						self.left.parent = self
				# else the key is not in the tree 
					
			else:					   # key should be in the right subtree
				if self.right:
					self.right = self.right.delete(line,lcmp,lon)
					if self.right:
						self.right.parent = self
					
		return self
	
	def _findMin(self, parent):
		""" return the minimum node in the current tree and its parent """

		# we use an ugly trick: the parent node is passed in as an argument
		# so that eventually when the leftmost child is reached, the 
		# call can return both the parent to the successor and the successor
		
		if self.left:
			return self.left._findMin(self)
		else:
			return [parent, self]

	def _findMax(self, parent):
		""" return the minimum node in the current tree and its parent """

		# we use an ugly trick: the parent node is passed in as an argument
		# so that eventually when the leftmost child is reached, the 
		# call can return both the parent to the successor and the successor
		
		if self.right:
			return self.right._findMin(self)
		else:
			return [parent, self]
